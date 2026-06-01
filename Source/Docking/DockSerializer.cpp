#include <RmlUi/Docking/DockSerializer.h>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

namespace RmlDock {
namespace {

const char* ToString(DockNodeKind kind)
{
    switch (kind) {
    case DockNodeKind::DockSpace:
        return "DockSpace";
    case DockNodeKind::Split:
        return "Split";
    case DockNodeKind::Leaf:
        return "Leaf";
    case DockNodeKind::Floating:
        return "Floating";
    }
    return "Leaf";
}

const char* ToString(DockAxis axis)
{
    return axis == DockAxis::Y ? "Y" : "X";
}

bool ParseKind(const std::string& value, DockNodeKind& kind)
{
    if (value == "DockSpace") {
        kind = DockNodeKind::DockSpace;
        return true;
    }
    if (value == "Split") {
        kind = DockNodeKind::Split;
        return true;
    }
    if (value == "Leaf") {
        kind = DockNodeKind::Leaf;
        return true;
    }
    if (value == "Floating") {
        kind = DockNodeKind::Floating;
        return true;
    }
    return false;
}

bool ParseAxis(const std::string& value, DockAxis& axis)
{
    if (value == "X") {
        axis = DockAxis::X;
        return true;
    }
    if (value == "Y") {
        axis = DockAxis::Y;
        return true;
    }
    return false;
}

bool IsUnescapedByte(unsigned char value)
{
    return std::isalnum(value) || value == '_' || value == '-' || value == '.';
}

std::string Escape(std::string_view text)
{
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0');
    for (const unsigned char value : text) {
        if (IsUnescapedByte(value)) {
            out << static_cast<char>(value);
        } else {
            out << '%' << std::setw(2) << static_cast<int>(value);
        }
    }
    return out.str();
}

int HexValue(char value)
{
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    return -1;
}

bool Unescape(std::string_view text, std::string& out)
{
    out.clear();
    out.reserve(text.size());
    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] != '%') {
            out.push_back(text[index]);
            continue;
        }

        if (index + 2 >= text.size()) {
            return false;
        }
        const int high = HexValue(text[index + 1]);
        const int low = HexValue(text[index + 2]);
        if (high < 0 || low < 0) {
            return false;
        }
        out.push_back(static_cast<char>((high << 4) | low));
        index += 2;
    }
    return true;
}

std::string EscapeTabs(const std::vector<PanelId>& tabs)
{
    std::ostringstream out;
    for (std::size_t index = 0; index < tabs.size(); ++index) {
        if (index != 0) {
            out << ',';
        }
        out << Escape(tabs[index]);
    }
    return out.str();
}

bool UnescapeTabs(const std::string& text, std::vector<PanelId>& tabs)
{
    tabs.clear();
    if (text.empty()) {
        return true;
    }

    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t comma = text.find(',', start);
        const std::size_t end = comma == std::string::npos ? text.size() : comma;
        std::string panel_id;
        if (!Unescape(std::string_view(text).substr(start, end - start), panel_id)) {
            return false;
        }
        tabs.push_back(std::move(panel_id));
        if (comma == std::string::npos) {
            break;
        }
        start = comma + 1;
    }
    return true;
}

bool ParseNodeField(const std::string& token, std::unordered_map<std::string, std::string>& fields)
{
    const std::size_t equals = token.find('=');
    if (equals == std::string::npos || equals == 0) {
        return false;
    }
    fields[token.substr(0, equals)] = token.substr(equals + 1);
    return true;
}

bool ParseDockNodeId(const std::unordered_map<std::string, std::string>& fields, const char* key, DockNodeId& value)
{
    const auto it = fields.find(key);
    if (it == fields.end()) {
        return false;
    }

    std::istringstream stream(it->second);
    std::uint32_t parsed = 0;
    stream >> parsed;
    if (!stream || !stream.eof()) {
        return false;
    }

    value = parsed;
    return true;
}

bool ParseFloat(const std::unordered_map<std::string, std::string>& fields, const char* key, float& value)
{
    const auto it = fields.find(key);
    if (it == fields.end()) {
        return false;
    }

    std::istringstream stream(it->second);
    stream >> value;
    return stream && stream.eof();
}

bool HasOnlyWhitespaceRemaining(std::istringstream& stream)
{
    stream >> std::ws;
    return stream.eof();
}

bool ParseNodeLine(const std::string& line, DockNode& node)
{
    std::istringstream stream(line);
    std::string label;
    stream >> label;
    if (label != "node") {
        return false;
    }

    std::unordered_map<std::string, std::string> fields;
    std::string token;
    while (stream >> token) {
        if (!ParseNodeField(token, fields)) {
            return false;
        }
    }

    if (!ParseDockNodeId(fields, "id", node.id)) {
        return false;
    }

    const auto kind_it = fields.find("kind");
    if (kind_it == fields.end() || !ParseKind(kind_it->second, node.kind)) {
        return false;
    }

    if (!ParseDockNodeId(fields, "parent", node.parent)
        || !ParseDockNodeId(fields, "child0", node.children[0])
        || !ParseDockNodeId(fields, "child1", node.children[1])
        || !ParseFloat(fields, "ratio", node.split_ratio)) {
        return false;
    }

    const auto axis_it = fields.find("axis");
    if (axis_it == fields.end() || !ParseAxis(axis_it->second, node.split_axis)) {
        return false;
    }

    const auto selected_it = fields.find("selected");
    if (selected_it == fields.end() || !Unescape(selected_it->second, node.selected_tab)) {
        return false;
    }

    const auto tabs_it = fields.find("tabs");
    if (tabs_it == fields.end() || !UnescapeTabs(tabs_it->second, node.tabs)) {
        return false;
    }

    return true;
}

void SetError(std::string* error, std::string value)
{
    if (error != nullptr) {
        *error = std::move(value);
    }
}

} // namespace

std::string SerializeDockTree(const DockTree& tree)
{
    std::vector<DockNodeId> node_ids;
    node_ids.reserve(tree.GetNodes().size());
    for (const auto& entry : tree.GetNodes()) {
        node_ids.push_back(entry.first);
    }
    std::sort(node_ids.begin(), node_ids.end());

    std::ostringstream out;
    out << "version 1\n";
    out << "root " << tree.GetRootNodeId() << '\n';

    for (const DockNodeId node_id : node_ids) {
        const DockNode& node = tree.GetNodes().at(node_id);
        out << "node id=" << node.id
            << " kind=" << ToString(node.kind)
            << " parent=" << node.parent
            << " child0=" << node.children[0]
            << " child1=" << node.children[1]
            << " axis=" << ToString(node.split_axis)
            << " ratio=" << node.split_ratio
            << " selected=" << Escape(node.selected_tab)
            << " tabs=" << EscapeTabs(node.tabs)
            << '\n';
    }

    return out.str();
}

bool DeserializeDockTree(std::string_view text, DockTree& out_tree, std::string* error)
{
    std::istringstream input{std::string(text)};
    std::string line;
    DockNodeId root_node_id = InvalidDockNodeId;
    std::unordered_map<DockNodeId, DockNode> nodes;
    DockNodeId max_node_id = InvalidDockNodeId;

    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream line_stream(line);
        std::string label;
        line_stream >> label;
        if (label == "version") {
            int version = 0;
            line_stream >> version;
            if (!line_stream || version != 1 || !HasOnlyWhitespaceRemaining(line_stream)) {
                SetError(error, "unsupported layout version");
                return false;
            }
        } else if (label == "root") {
            line_stream >> root_node_id;
            if (!line_stream || !HasOnlyWhitespaceRemaining(line_stream)) {
                SetError(error, "invalid root node");
                return false;
            }
        } else if (label == "node") {
            DockNode node;
            if (!ParseNodeLine(line, node)) {
                SetError(error, "invalid node line");
                return false;
            }
            if (nodes.find(node.id) != nodes.end()) {
                SetError(error, "invalid node line");
                return false;
            }
            max_node_id = std::max(max_node_id, node.id);
            nodes[node.id] = std::move(node);
        }
    }

    if (!out_tree.SetFromNodes(root_node_id, max_node_id + 1, std::move(nodes))) {
        SetError(error, "invalid root node");
        return false;
    }

    if (error != nullptr) {
        error->clear();
    }
    return true;
}

}
