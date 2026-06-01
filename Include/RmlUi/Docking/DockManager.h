#pragma once

#include <RmlUi/Docking/DockTree.h>

#include <optional>

namespace RmlDock {

class DockManager {
public:
    DockHit CalculateHit(DockNodeId target_node, DockRect target_rect, float mouse_x, float mouse_y) const;

    void SetPreview(DockHit hit) { preview_hit_ = hit; }
    void ClearPreview() { preview_hit_.reset(); }
    const std::optional<DockHit>& GetPreview() const { return preview_hit_; }

    bool ApplyRequest(DockTree& tree, const DockRequest& request) const;

private:
    std::optional<DockHit> preview_hit_;
};

}
