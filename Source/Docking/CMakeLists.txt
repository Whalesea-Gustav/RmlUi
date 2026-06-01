add_library(rmlui_docking STATIC
	DockBuilder.cpp
	DockInteraction.cpp
	DockManager.cpp
	DockSerializer.cpp
	DockTree.cpp
	PanelRegistry.cpp
	RmlDockRenderer.cpp
)

set_common_target_options(rmlui_docking)

target_include_directories(rmlui_docking PRIVATE "${PROJECT_SOURCE_DIR}/Include")
target_include_directories(rmlui_docking INTERFACE
	"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Include>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_sources(rmlui_docking PRIVATE
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/DockBuilder.h"
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/DockInteraction.h"
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/DockManager.h"
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/DockPolicy.h"
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/DockSerializer.h"
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/DockTree.h"
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/DockTypes.h"
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/PanelRegistry.h"
	"${PROJECT_SOURCE_DIR}/Include/RmlUi/Docking/RmlDockRenderer.h"
)

target_link_libraries(rmlui_docking PUBLIC rmlui_core)

add_library(RmlUi::Docking ALIAS rmlui_docking)

set_target_properties(rmlui_docking PROPERTIES
	EXPORT_NAME "Docking"
	OUTPUT_NAME "rmlui_docking"
)

install(TARGETS rmlui_docking
	EXPORT RmlUiTargets
	${RMLUI_RUNTIME_DEPENDENCY_SET_ARG}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
