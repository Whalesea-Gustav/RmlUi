#pragma once

#include <RmlUi/Docking/DockTypes.h>

#include <array>
#include <cstddef>

namespace RmlDock {

constexpr std::size_t DockDropZoneCount = static_cast<std::size_t>(DockDropZone::Count);

struct DockPolicy {
    bool allow_split = true;
    bool allow_tabs = true;
    bool force_tabs = false;
    bool allow_floating = true;
    bool allow_close = true;
    bool allow_resize = true;
    std::array<bool, DockDropZoneCount> allowed_drop_zones = {true, true, true, true, true};

    bool Allows(DockDropZone zone) const
    {
        const int index = static_cast<int>(zone);
        if (index < 0 || static_cast<std::size_t>(index) >= allowed_drop_zones.size()) {
            return false;
        }

        return allowed_drop_zones[static_cast<std::size_t>(index)];
    }
};

}
