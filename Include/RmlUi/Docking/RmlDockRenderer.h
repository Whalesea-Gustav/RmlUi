#pragma once

#include <RmlUi/Docking/DockManager.h>
#include <RmlUi/Docking/DockTree.h>
#include <RmlUi/Docking/PanelRegistry.h>

namespace Rml {
class Element;
class ElementDocument;
}

namespace RmlDock {

class RmlDockRenderer {
public:
    bool Render(Rml::ElementDocument& document, const char* root_element_id, const DockTree& tree, const PanelRegistry* registry, const DockManager* manager);

private:
    void ClearChildren(Rml::Element& element);
    void RenderNode(Rml::ElementDocument& document, Rml::Element& parent, const DockTree& tree, const DockNode& node, const PanelRegistry* registry, const DockManager* manager);
    void RenderLeaf(Rml::ElementDocument& document, Rml::Element& parent, const DockNode& node, const PanelRegistry* registry, const DockManager* manager);
    void RenderSplit(Rml::ElementDocument& document, Rml::Element& parent, const DockTree& tree, const DockNode& node, const PanelRegistry* registry, const DockManager* manager);
};

}
