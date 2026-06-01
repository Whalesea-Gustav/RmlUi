#include <RmlUi/Docking/DockManager.h>

namespace RmlDock {

DockHit DockManager::CalculateHit(DockNodeId target_node, DockRect rect, float mouse_x, float mouse_y) const
{
    if (target_node == InvalidDockNodeId || rect.width <= 0.0f || rect.height <= 0.0f) {
        return {};
    }

    if (mouse_x < rect.x || mouse_y < rect.y || mouse_x > rect.x + rect.width || mouse_y > rect.y + rect.height) {
        return {};
    }

    const float local_x = (mouse_x - rect.x) / rect.width;
    const float local_y = (mouse_y - rect.y) / rect.height;

    DockDropZone zone = DockDropZone::Center;
    if (local_x < 0.25f) {
        zone = DockDropZone::Left;
    } else if (local_x > 0.75f) {
        zone = DockDropZone::Right;
    } else if (local_y < 0.25f) {
        zone = DockDropZone::Top;
    } else if (local_y > 0.75f) {
        zone = DockDropZone::Bottom;
    }

    return DockHit{target_node, zone, true};
}

bool DockManager::ApplyRequest(DockTree& tree, const DockRequest& request) const
{
    return tree.ApplyDockRequest(request);
}

}
