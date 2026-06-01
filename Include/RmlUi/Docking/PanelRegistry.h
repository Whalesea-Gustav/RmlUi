#pragma once

#include <RmlUi/Docking/DockPolicy.h>

#include <string>
#include <unordered_map>

namespace RmlDock {

struct PanelDescriptor {
    PanelId id;
    std::string title;
    DockPolicy policy;
};

class PanelRegistry {
public:
    bool RegisterPanel(PanelDescriptor descriptor);
    const PanelDescriptor* FindPanel(const PanelId& id) const;

private:
    std::unordered_map<PanelId, PanelDescriptor> panels_;
};

}
