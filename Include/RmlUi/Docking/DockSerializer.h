#pragma once

#include <RmlUi/Docking/DockTree.h>

#include <string>
#include <string_view>

namespace RmlDock {

std::string SerializeDockTree(const DockTree& tree);
bool DeserializeDockTree(std::string_view text, DockTree& out_tree, std::string* error);

}
