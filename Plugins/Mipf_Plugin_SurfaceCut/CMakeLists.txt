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

project(Mipf_Plugin_SurfaceCut)
file(GLOB Headers "./*.h")
file(GLOB UI_Srcs "./*.ui")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

include_directories(${MitkCoreExt_INCLUDE_DIR})

add_definitions(-DUS_MODULE_NAME)
add_definitions(-DQWT_DLL)

set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE ColorEdit_DIR_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/ColorEdit/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/ColorEdit/*.c* ) 

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ColorEdit)
source_group(ColorEdit FILES  ${ColorEdit_DIR_SRC})


if (VTK_QT_VERSION VERSION_GREATER "4")
    qt5_wrap_ui(ui_Files ${UI_Srcs})
    add_library(Mipf_Plugin_SurfaceCut SHARED ${DIR_SRCS} ${ColorEdit_DIR_SRC} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
    qt5_use_modules(Mipf_Plugin_SurfaceCut Core Gui Widgets)
    target_link_libraries(Mipf_Plugin_SurfaceCut    
    ${VTK_LIBRARIES} ${ITK_LIBRARIES}   
    ${MitkCoreExt_LIBRARIES}  
    MitkQtWidgets MitkQtWidgetsExt MitkBoundingShape)
	link_framework()
else()
    add_library(Mipf_Plugin_SurfaceCut SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
    target_link_libraries(Mipf_Plugin_SurfaceCut ${QT_LIBRARIES@} ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt)

endif()

					  


