#include <RmlUi/Docking/PanelRegistry.h>

#include <utility>

namespace RmlDock {

bool PanelRegistry::RegisterPanel(PanelDescriptor descriptor)
{
    if (descriptor.id.empty() || panels_.find(descriptor.id) != panels_.end()) {
        return false;
    }
    panels_.emplace(descriptor.id, std::move(descriptor));
    return true;
}

const PanelDescriptor* PanelRegistry::FindPanel(const PanelId& id) const
{
    const auto it = panels_.find(id);
    return it == panels_.end() ? nullptr : &it->second;
}

}
