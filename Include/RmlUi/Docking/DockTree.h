#pragma once

#include <RmlUi/Docking/DockTypes.h>

#include <unordered_map>

namespace RmlDock {

class DockTree {
public:
    DockNodeId CreateDockSpace();
    DockNodeId AddPanelToRoot(PanelId panel_id);
    bool SelectPanel(const PanelId& panel_id);
    bool ApplyDockRequest(const DockRequest& request);
    bool MovePanelToLeaf(const PanelId& panel_id, DockNodeId leaf_node_id);
    bool SplitLeaf(DockNodeId target_node_id, DockDropZone zone, PanelId payload_panel_id);
    bool SetSplitRatio(DockNodeId split_node, float ratio);
    bool RemovePanel(const PanelId& panel_id);
    void NormalizeTree();

    DockNodeId GetRootNodeId() const;

    DockNode* FindNode(DockNodeId node_id);
    const DockNode* FindNode(DockNodeId node_id) const;

    DockNode* FindPanelNode(const PanelId& panel_id);
    const DockNode* FindPanelNode(const PanelId& panel_id) const;

    const std::unordered_map<DockNodeId, DockNode>& GetNodes() const { return nodes_; }
    bool SetFromNodes(DockNodeId root_node_id, DockNodeId next_node_id, std::unordered_map<DockNodeId, DockNode> nodes);

private:
    DockNodeId CreateNode(DockNodeKind kind);
    DockNodeId CreateLeafWithPanel(PanelId panel_id);
    DockNodeId FindFirstLeaf(DockNodeId start_node_id) const;
    void ReplaceChild(DockNodeId parent_node_id, DockNodeId old_child_node_id, DockNodeId new_child_node_id);
    void RemoveNode(DockNodeId node_id);
    bool IsLeaf(DockNodeId node_id) const;
    bool DetachPanelWithoutNormalize(const PanelId& panel_id);

    DockNodeId root_node_id_ = InvalidDockNodeId;
    DockNodeId next_node_id_ = 1;
    std::unordered_map<DockNodeId, DockNode> nodes_;
};

}
