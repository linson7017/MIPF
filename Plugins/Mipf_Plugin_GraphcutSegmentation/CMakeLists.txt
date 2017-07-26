cmake_minimum_required(VERSION 2.8.11)
if (POLICY CMP0020)
cmake_policy(SET CMP0020 NEW)
endif()
if (POLICY CMP0025)
cmake_policy(SET CMP0025 NEW) # CMake 3.0
endif()
if (POLICY CMP0043)
cmake_policy(SET CMP0043 NEW) # CMake 3.0
endif()
if (POLICY CMP0053)
cmake_policy(SET CMP0053 NEW) # CMake 3.1
endif()

project(Mipf_Plugin_GraphcutSegmentation)
file(GLOB Headers "./*.h" "*.hpp")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
AUX_SOURCE_DIRECTORY(./maxflow GRAPH_CUT_SRCS)
SET(SRCS ${DIR_SRCS} ${GRAPH_CUT_SRCS} ${Headers})

include_directories(${ITKVTK_HELPERS_INCLUDE_DIR})

set(CMAKE_AUTOMOC ON)

if (VTK_QT_VERSION VERSION_GREATER "4")

    add_library(Mipf_Plugin_GraphcutSegmentation SHARED ${SRCS} ${Headers}  ${UI_Srcs} ${MOC_Hdrs})
    qt5_use_modules(Mipf_Plugin_GraphcutSegmentation Core Gui Widgets Concurrent)
    target_link_libraries(Mipf_Plugin_GraphcutSegmentation ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} ${ITKVTK_HELPERS_LIBRARIES}
	MitkQtWidgets MitkQtWidgetsExt MitkSegmentation )

else()
    add_library(Mipf_Plugin_GraphcutSegmentation SHARED ${SRCS} ${Headers}  ${UI_Srcs} ${MOC_Hdrs} )
    target_link_libraries(Mipf_Plugin_GraphcutSegmentation ${QT_LIBRARIES@} ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} ${ITKVTK_HELPERS_LIBRARIES}
	MitkQtWidgets MitkQtWidgetsExt MitkSegmentation)

endif()
