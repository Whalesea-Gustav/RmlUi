#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace RmlDock {

using DockNodeId = std::uint32_t;
using PanelId = std::string;

constexpr DockNodeId InvalidDockNodeId = 0;

enum class DockNodeKind {
    DockSpace,
    Split,
    Leaf,
    Floating,
};

enum class DockAxis {
    X,
    Y,
};

enum class DockDropZone {
    Center,
    Left,
    Right,
    Top,
    Bottom,
    Count,
};

struct DockRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct DockNode {
    DockNodeId id = InvalidDockNodeId;
    DockNodeKind kind = DockNodeKind::Leaf;
    DockNodeId parent = InvalidDockNodeId;
    std::array<DockNodeId, 2> children = {InvalidDockNodeId, InvalidDockNodeId};
    DockAxis split_axis = DockAxis::X;
    float split_ratio = 0.5f;
    std::vector<PanelId> tabs;
    PanelId selected_tab;
    DockRect floating_rect;
};

struct DockRequest {
    PanelId payload_panel;
    DockNodeId target_node = InvalidDockNodeId;
    DockDropZone zone = DockDropZone::Center;
};

struct DockHit {
    DockNodeId target_node = InvalidDockNodeId;
    DockDropZone zone = DockDropZone::Center;
    bool valid = false;
};

}
