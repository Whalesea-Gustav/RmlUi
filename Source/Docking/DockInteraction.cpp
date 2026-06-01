#include <RmlUi/Docking/DockInteraction.h>

#include <algorithm>
#include <cmath>

namespace RmlDock {

namespace {

float ClampSplitRatio(float ratio)
{
    if (!std::isfinite(ratio)) {
        return 0.5f;
    }
    return std::clamp(ratio, 0.05f, 0.95f);
}

}

float CalculateSplitRatioFromMouse(const DockNode& split_node, DockRect split_rect, float mouse_x, float mouse_y)
{
    if (split_node.kind != DockNodeKind::Split) {
        return ClampSplitRatio(split_node.split_ratio);
    }

    const bool horizontal_split = split_node.split_axis == DockAxis::X;
    const float length = horizontal_split ? split_rect.width : split_rect.height;
    if (length <= 0.0f || !std::isfinite(length)) {
        return ClampSplitRatio(split_node.split_ratio);
    }

    const float origin = horizontal_split ? split_rect.x : split_rect.y;
    const float mouse_position = horizontal_split ? mouse_x : mouse_y;
    return ClampSplitRatio((mouse_position - origin) / length);
}

DockRequest MakeDockRequest(PanelId payload_panel, DockHit hit)
{
    if (payload_panel.empty() || !hit.valid) {
        return {};
    }

    return DockRequest{std::move(payload_panel), hit.target_node, hit.zone};
}

}
