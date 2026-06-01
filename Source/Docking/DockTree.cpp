#include <RmlUi/Docking/DockTree.h>

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace RmlDock {

DockNodeId DockTree::CreateDockSpace()
{
    nodes_.clear();
    root_node_id_ = InvalidDockNodeId;
    next_node_id_ = 1;

    const DockNodeId root_id = CreateNode(DockNodeKind::DockSpace);
    const DockNodeId leaf_id = CreateNode(DockNodeKind::Leaf);

    DockNode* root_node = FindNode(root_id);
    DockNode* leaf_node = FindNode(leaf_id);
    root_node->children[0] = leaf_id;
    leaf_node->parent = root_id;
    root_node_id_ = root_id;

    return root_id;
}

DockNodeId DockTree::AddPanelToRoot(PanelId panel_id)
{
    if (root_node_id_ == InvalidDockNodeId) {
        CreateDockSpace();
    }

    const DockNodeId leaf_id = FindFirstLeaf(root_node_id_);
    DockNode* leaf = FindNode(leaf_id);
    if (leaf == nullptr) {
        return InvalidDockNodeId;
    }

    if (std::find(leaf->tabs.begin(), leaf->tabs.end(), panel_id) == leaf->tabs.end()) {
        leaf->tabs.push_back(panel_id);
    }
    leaf->selected_tab = std::move(panel_id);

    return leaf_id;
}

bool DockTree::SelectPanel(const PanelId& panel_id)
{
    DockNode* node = FindPanelNode(panel_id);
    if (node == nullptr) {
        return false;
    }

    node->selected_tab = panel_id;
    return true;
}

bool DockTree::ApplyDockRequest(const DockRequest& request)
{
    if (request.payload_panel.empty() || FindNode(request.target_node) == nullptr) {
        return false;
    }

    if (request.zone == DockDropZone::Center) {
        return MovePanelToLeaf(request.payload_panel, request.target_node);
    }

    return SplitLeaf(request.target_node, request.zone, request.payload_panel);
}

bool DockTree::MovePanelToLeaf(const PanelId& panel_id, DockNodeId leaf_node_id)
{
    if (!IsLeaf(leaf_node_id)) {
        return false;
    }

    DockNode* leaf = FindNode(leaf_node_id);
    if (std::find(leaf->tabs.begin(), leaf->tabs.end(), panel_id) != leaf->tabs.end()) {
        leaf->selected_tab = panel_id;
        return true;
    }

    if (!DetachPanelWithoutNormalize(panel_id)) {
        return false;
    }

    leaf = FindNode(leaf_node_id);
    if (leaf == nullptr || leaf->kind != DockNodeKind::Leaf) {
        NormalizeTree();
        return false;
    }

    if (std::find(leaf->tabs.begin(), leaf->tabs.end(), panel_id) == leaf->tabs.end()) {
        leaf->tabs.push_back(panel_id);
    }
    leaf->selected_tab = panel_id;
    NormalizeTree();
    return true;
}

bool DockTree::SplitLeaf(DockNodeId target_node_id, DockDropZone zone, PanelId payload_panel_id)
{
    if (!IsLeaf(target_node_id)) {
        return false;
    }

    if (zone != DockDropZone::Left && zone != DockDropZone::Right && zone != DockDropZone::Top && zone != DockDropZone::Bottom) {
        return false;
    }

    const DockNode* source = FindPanelNode(payload_panel_id);
    if (source == nullptr) {
        return false;
    }
    if (source->id == target_node_id && source->tabs.size() == 1) {
        return false;
    }

    if (!DetachPanelWithoutNormalize(payload_panel_id)) {
        return false;
    }

    DockNode* target = FindNode(target_node_id);
    if (target == nullptr || target->kind != DockNodeKind::Leaf) {
        NormalizeTree();
        return false;
    }

    const DockNodeId old_parent_id = target->parent;
    const DockNodeId split_id = CreateNode(DockNodeKind::Split);
    const DockNodeId payload_leaf_id = CreateLeafWithPanel(std::move(payload_panel_id));

    DockNode* split = FindNode(split_id);
    target = FindNode(target_node_id);
    DockNode* payload_leaf = FindNode(payload_leaf_id);
    if (split == nullptr || target == nullptr || payload_leaf == nullptr) {
        NormalizeTree();
        return false;
    }

    split->parent = old_parent_id;
    split->split_axis = (zone == DockDropZone::Left || zone == DockDropZone::Right) ? DockAxis::X : DockAxis::Y;
    split->split_ratio = 0.5f;

    const bool payload_first = zone == DockDropZone::Left || zone == DockDropZone::Top;
    split->children[0] = payload_first ? payload_leaf_id : target_node_id;
    split->children[1] = payload_first ? target_node_id : payload_leaf_id;
    target->parent = split_id;
    payload_leaf->parent = split_id;

    if (old_parent_id != InvalidDockNodeId) {
        ReplaceChild(old_parent_id, target_node_id, split_id);
    } else {
        root_node_id_ = split_id;
    }

    NormalizeTree();
    return true;
}

bool DockTree::SetSplitRatio(DockNodeId split_node, float ratio)
{
    DockNode* node = FindNode(split_node);
    if (!node || node->kind != DockNodeKind::Split) {
        return false;
    }
    if (!std::isfinite(ratio)) {
        return false;
    }
    if (ratio < 0.05f) {
        ratio = 0.05f;
    }
    if (ratio > 0.95f) {
        ratio = 0.95f;
    }
    node->split_ratio = ratio;
    return true;
}

bool DockTree::RemovePanel(const PanelId& panel_id)
{
    if (!DetachPanelWithoutNormalize(panel_id)) {
        return false;
    }

    NormalizeTree();
    return true;
}

void DockTree::NormalizeTree()
{
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<DockNodeId> split_node_ids;
        split_node_ids.reserve(nodes_.size());
        for (const auto& entry : nodes_) {
            if (entry.second.kind == DockNodeKind::Split) {
                split_node_ids.push_back(entry.first);
            }
        }

        for (const DockNodeId split_node_id : split_node_ids) {
            DockNode* split = FindNode(split_node_id);
            if (split == nullptr) {
                continue;
            }

            const DockNodeId first_child_id = split->children[0];
            const DockNodeId second_child_id = split->children[1];
            const DockNode* first_child = FindNode(first_child_id);
            const DockNode* second_child = FindNode(second_child_id);
            const bool first_empty_leaf = first_child != nullptr && first_child->kind == DockNodeKind::Leaf && first_child->tabs.empty();
            const bool second_empty_leaf = second_child != nullptr && second_child->kind == DockNodeKind::Leaf && second_child->tabs.empty();
            if (first_empty_leaf == second_empty_leaf) {
                continue;
            }

            const DockNodeId empty_leaf_id = first_empty_leaf ? first_child_id : second_child_id;
            const DockNodeId sibling_id = first_empty_leaf ? second_child_id : first_child_id;
            const DockNodeId grandparent_id = split->parent;
            DockNode* sibling = FindNode(sibling_id);
            if (sibling == nullptr) {
                continue;
            }

            sibling->parent = grandparent_id;
            if (grandparent_id != InvalidDockNodeId) {
                ReplaceChild(grandparent_id, split_node_id, sibling_id);
            } else {
                root_node_id_ = sibling_id;
            }

            RemoveNode(empty_leaf_id);
            RemoveNode(split_node_id);
            changed = true;
            break;
        }
    }
}

DockNodeId DockTree::GetRootNodeId() const
{
    return root_node_id_;
}

DockNode* DockTree::FindNode(DockNodeId node_id)
{
    const auto it = nodes_.find(node_id);
    if (it == nodes_.end()) {
        return nullptr;
    }
    return &it->second;
}

const DockNode* DockTree::FindNode(DockNodeId node_id) const
{
    const auto it = nodes_.find(node_id);
    if (it == nodes_.end()) {
        return nullptr;
    }
    return &it->second;
}

DockNode* DockTree::FindPanelNode(const PanelId& panel_id)
{
    for (auto& entry : nodes_) {
        DockNode& node = entry.second;
        if (std::find(node.tabs.begin(), node.tabs.end(), panel_id) != node.tabs.end()) {
            return &node;
        }
    }
    return nullptr;
}

const DockNode* DockTree::FindPanelNode(const PanelId& panel_id) const
{
    for (const auto& entry : nodes_) {
        const DockNode& node = entry.second;
        if (std::find(node.tabs.begin(), node.tabs.end(), panel_id) != node.tabs.end()) {
            return &node;
        }
    }
    return nullptr;
}

bool DockTree::SetFromNodes(DockNodeId root_node_id, DockNodeId next_node_id, std::unordered_map<DockNodeId, DockNode> nodes)
{
    if (root_node_id == InvalidDockNodeId || nodes.find(root_node_id) == nodes.end()) {
        return false;
    }
    if (next_node_id == InvalidDockNodeId) {
        return false;
    }

    DockNodeId max_node_id = InvalidDockNodeId;
    for (const auto& entry : nodes) {
        const DockNodeId node_id = entry.first;
        const DockNode& node = entry.second;
        if (node_id == InvalidDockNodeId || node.id == InvalidDockNodeId || node_id != node.id) {
            return false;
        }
        max_node_id = std::max(max_node_id, node_id);
    }
    if (next_node_id <= max_node_id) {
        return false;
    }

    const DockNode& root = nodes.find(root_node_id)->second;
    if (root.parent != InvalidDockNodeId) {
        return false;
    }

    for (const auto& entry : nodes) {
        const DockNodeId node_id = entry.first;
        const DockNode& node = entry.second;

        if (node_id != root_node_id && (node.parent == InvalidDockNodeId || nodes.find(node.parent) == nodes.end())) {
            return false;
        }

        for (const DockNodeId child_id : node.children) {
            if (child_id == InvalidDockNodeId) {
                continue;
            }

            const auto child_it = nodes.find(child_id);
            if (child_it == nodes.end() || child_it->second.parent != node_id) {
                return false;
            }
        }

        if (node.kind == DockNodeKind::Split) {
            if (node.children[0] == InvalidDockNodeId || node.children[1] == InvalidDockNodeId) {
                return false;
            }
        } else if (node.kind == DockNodeKind::Leaf) {
            if (node.children[0] != InvalidDockNodeId || node.children[1] != InvalidDockNodeId) {
                return false;
            }
        } else if (node.kind == DockNodeKind::DockSpace) {
            if (node.children[1] != InvalidDockNodeId) {
                return false;
            }
        } else if (node.kind == DockNodeKind::Floating) {
            return false;
        }
    }

    root_node_id_ = root_node_id;
    next_node_id_ = next_node_id;
    nodes_ = std::move(nodes);
    return true;
}

DockNodeId DockTree::CreateNode(DockNodeKind kind)
{
    DockNode node;
    node.id = next_node_id_++;
    node.kind = kind;

    const DockNodeId node_id = node.id;
    nodes_.emplace(node_id, std::move(node));
    return node_id;
}

DockNodeId DockTree::CreateLeafWithPanel(PanelId panel_id)
{
    const DockNodeId leaf_id = CreateNode(DockNodeKind::Leaf);
    DockNode* leaf = FindNode(leaf_id);
    if (leaf == nullptr) {
        return InvalidDockNodeId;
    }

    leaf->tabs.push_back(panel_id);
    leaf->selected_tab = std::move(panel_id);
    return leaf_id;
}

DockNodeId DockTree::FindFirstLeaf(DockNodeId start_node_id) const
{
    const DockNode* node = FindNode(start_node_id);
    if (node == nullptr) {
        return InvalidDockNodeId;
    }

    if (node->kind == DockNodeKind::Leaf) {
        return start_node_id;
    }

    for (const DockNodeId child_id : node->children) {
        const DockNodeId leaf_id = FindFirstLeaf(child_id);
        if (leaf_id != InvalidDockNodeId) {
            return leaf_id;
        }
    }

    return InvalidDockNodeId;
}

void DockTree::ReplaceChild(DockNodeId parent_node_id, DockNodeId old_child_node_id, DockNodeId new_child_node_id)
{
    DockNode* parent = FindNode(parent_node_id);
    if (parent == nullptr) {
        return;
    }

    for (DockNodeId& child_node_id : parent->children) {
        if (child_node_id == old_child_node_id) {
            child_node_id = new_child_node_id;
            return;
        }
    }
}

void DockTree::RemoveNode(DockNodeId node_id)
{
    nodes_.erase(node_id);
}

bool DockTree::IsLeaf(DockNodeId node_id) const
{
    const DockNode* node = FindNode(node_id);
    return node != nullptr && node->kind == DockNodeKind::Leaf;
}

bool DockTree::DetachPanelWithoutNormalize(const PanelId& panel_id)
{
    DockNode* node = FindPanelNode(panel_id);
    if (node == nullptr) {
        return false;
    }

    const auto tab_it = std::find(node->tabs.begin(), node->tabs.end(), panel_id);
    if (tab_it == node->tabs.end()) {
        return false;
    }

    const bool selected = node->selected_tab == panel_id;
    node->tabs.erase(tab_it);
    if (selected) {
        node->selected_tab = node->tabs.empty() ? PanelId{} : node->tabs.front();
    }

    return true;
}

}
