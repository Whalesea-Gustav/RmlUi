#include <RmlUi/Docking/DockBuilder.h>

namespace RmlDock {

DockTree BuildDefaultEditorDockTree()
{
    DockTree tree;
    tree.CreateDockSpace();
    const DockNodeId root_leaf = tree.AddPanelToRoot("Scene");
    tree.AddPanelToRoot("Game");
    tree.AddPanelToRoot("Hierarchy");
    tree.ApplyDockRequest({"Hierarchy", root_leaf, DockDropZone::Left});
    tree.AddPanelToRoot("Inspector");
    tree.ApplyDockRequest({"Inspector", root_leaf, DockDropZone::Right});
    tree.AddPanelToRoot("Console");
    tree.AddPanelToRoot("Output");
    tree.ApplyDockRequest({"Console", root_leaf, DockDropZone::Bottom});
    tree.ApplyDockRequest({"Output", tree.FindPanelNode("Console")->id, DockDropZone::Center});
    return tree;
}

}
