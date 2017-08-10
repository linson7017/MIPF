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

project(Mipf_Plugin_VMTKTools)
file(GLOB Headers "./*.h")
file(GLOB UI_Srcs "./*.ui")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

set(CMAKE_AUTOMOC ON)

include_directories(${VMTK_INCLUDE_DIR})
include_directories(${ITKVTK_HELPERS_INCLUDE_DIR})

if (VTK_QT_VERSION VERSION_GREATER "4")
    qt5_wrap_ui(ui_Files ${UI_Srcs})
    add_library(Mipf_Plugin_VMTKTools SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
    qt5_use_modules(Mipf_Plugin_VMTKTools Core Gui Widgets)
    target_link_libraries(Mipf_Plugin_VMTKTools 
    ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${VMTK_LIBRARIES} ${ITKVTK_HELPERS_LIBRARIES}
    ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} 
    MitkQtWidgets MitkQtWidgetsExt)

else()
    add_library(Mipf_Plugin_VMTKTools SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
    target_link_libraries(Mipf_Plugin_VMTKTools ${QT_LIBRARIES@} ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt)

endif()
