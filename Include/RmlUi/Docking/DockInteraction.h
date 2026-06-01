#pragma once

#include <RmlUi/Docking/DockTypes.h>

namespace RmlDock {

float CalculateSplitRatioFromMouse(const DockNode& split_node, DockRect split_rect, float mouse_x, float mouse_y);
DockRequest MakeDockRequest(PanelId payload_panel, DockHit hit);

}
