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

project(Mipf_Plugin_VolumeCut)
file(GLOB Headers "./*.h")
file(GLOB UI_Srcs "./*.ui")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

include_directories(${MitkCoreExt_INCLUDE_DIR})

set(CMAKE_AUTOMOC ON)

qt5_wrap_ui(ui_Files ${UI_Srcs})

add_library(Mipf_Plugin_VolumeCut SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})

qt5_use_modules(Mipf_Plugin_VolumeCut Core Gui Widgets)

target_link_libraries(Mipf_Plugin_VolumeCut    
${VTK_LIBRARIES} ${ITK_LIBRARIES}     
${MitkCoreExt_LIBRARIES}  
MitkQtWidgets MitkQtWidgetsExt MitkBoundingShape)
link_framework()

