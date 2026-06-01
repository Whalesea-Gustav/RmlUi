#include <RmlUi/Docking/RmlDockRenderer.h>

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/StringUtilities.h>

#include <utility>

namespace RmlDock {

namespace {

Rml::String NodeIdString(DockNodeId node_id)
{
    return Rml::CreateString("%u", node_id);
}

const char* DropZoneClass(DockDropZone zone)
{
    switch (zone) {
    case DockDropZone::Center:
        return "dock-preview-center";
    case DockDropZone::Left:
        return "dock-preview-left";
    case DockDropZone::Right:
        return "dock-preview-right";
    case DockDropZone::Top:
        return "dock-preview-top";
    case DockDropZone::Bottom:
        return "dock-preview-bottom";
    case DockDropZone::Count:
        break;
    }

    return "";
}

Rml::Element* AppendElement(Rml::ElementDocument& document, Rml::Element& parent, const Rml::String& tag)
{
    Rml::ElementPtr element = document.CreateElement(tag);
    return parent.AppendChild(std::move(element));
}

void ApplySplitChildFlex(Rml::Element* element, float ratio)
{
    if (element == nullptr) {
        return;
    }

    element->SetProperty("flex", Rml::CreateString("%.4f 1 0", ratio));
}

} // namespace

bool RmlDockRenderer::Render(Rml::ElementDocument& document, const char* root_element_id, const DockTree& tree, const PanelRegistry* registry, const DockManager* manager)
{
    Rml::Element* root = document.GetElementById(root_element_id);
    if (root == nullptr) {
        return false;
    }

    ClearChildren(*root);
    root->SetClass("dock-root", true);

    const DockNode* root_node = tree.FindNode(tree.GetRootNodeId());
    if (root_node == nullptr) {
        return false;
    }

    RenderNode(document, *root, tree, *root_node, registry, manager);
    return true;
}

void RmlDockRenderer::ClearChildren(Rml::Element& element)
{
    while (element.GetNumChildren() > 0) {
        element.RemoveChild(element.GetChild(0));
    }
}

void RmlDockRenderer::RenderNode(Rml::ElementDocument& document, Rml::Element& parent, const DockTree& tree, const DockNode& node, const PanelRegistry* registry, const DockManager* manager)
{
    switch (node.kind) {
    case DockNodeKind::DockSpace:
        for (const DockNodeId child_id : node.children) {
            if (const DockNode* child = tree.FindNode(child_id)) {
                RenderNode(document, parent, tree, *child, registry, manager);
            }
        }
        break;
    case DockNodeKind::Split:
        RenderSplit(document, parent, tree, node, registry, manager);
        break;
    case DockNodeKind::Leaf:
        RenderLeaf(document, parent, node, registry, manager);
        break;
    case DockNodeKind::Floating:
        break;
    }
}

void RmlDockRenderer::RenderLeaf(Rml::ElementDocument& document, Rml::Element& parent, const DockNode& node, const PanelRegistry* registry, const DockManager* manager)
{
    Rml::Element* leaf = AppendElement(document, parent, "div");
    leaf->SetClass("dock-leaf", true);
    leaf->SetAttribute("data-node", NodeIdString(node.id));
    if (manager != nullptr) {
        const std::optional<DockHit>& preview = manager->GetPreview();
        if (preview.has_value() && preview->valid && preview->target_node == node.id) {
            leaf->SetClass("dock-preview", true);
            leaf->SetClass(DropZoneClass(preview->zone), true);
        }
    }

    Rml::Element* tabs = AppendElement(document, *leaf, "div");
    tabs->SetClass("dock-tabs", true);

    for (const PanelId& panel_id : node.tabs) {
        Rml::Element* tab = AppendElement(document, *tabs, "button");
        tab->SetClass("dock-tab", true);
        tab->SetClass("active", panel_id == node.selected_tab);
        tab->SetAttribute("data-panel", panel_id);

        const PanelDescriptor* descriptor = registry != nullptr ? registry->FindPanel(panel_id) : nullptr;
        const Rml::String title = descriptor != nullptr && !descriptor->title.empty() ? descriptor->title : panel_id;
        tab->SetInnerRML(Rml::StringUtilities::EncodeRml(title));
    }

    Rml::Element* content = AppendElement(document, *leaf, "div");
    content->SetClass("dock-content", true);
    content->SetAttribute("data-panel-host", node.selected_tab);
    content->SetInnerRML(Rml::StringUtilities::EncodeRml(node.selected_tab));
}

void RmlDockRenderer::RenderSplit(Rml::ElementDocument& document, Rml::Element& parent, const DockTree& tree, const DockNode& node, const PanelRegistry* registry, const DockManager* manager)
{
    Rml::Element* split = AppendElement(document, parent, "div");
    split->SetClass("dock-split", true);
    split->SetClass(node.split_axis == DockAxis::X ? "dock-split-x" : "dock-split-y", true);
    split->SetAttribute("data-node", NodeIdString(node.id));
    split->SetAttribute("data-ratio", Rml::CreateString("%.3f", node.split_ratio));

    if (const DockNode* first_child = tree.FindNode(node.children[0])) {
        const int child_index = split->GetNumChildren();
        RenderNode(document, *split, tree, *first_child, registry, manager);
        if (split->GetNumChildren() > child_index) {
            ApplySplitChildFlex(split->GetChild(child_index), node.split_ratio);
        }
    }

    Rml::Element* splitter = AppendElement(document, *split, "handle");
    splitter->SetClass("dock-splitter", true);
    splitter->SetAttribute("data-splitter", NodeIdString(node.id));

    if (const DockNode* second_child = tree.FindNode(node.children[1])) {
        const int child_index = split->GetNumChildren();
        RenderNode(document, *split, tree, *second_child, registry, manager);
        if (split->GetNumChildren() > child_index) {
            ApplySplitChildFlex(split->GetChild(child_index), 1.0f - node.split_ratio);
        }
    }
}

}
