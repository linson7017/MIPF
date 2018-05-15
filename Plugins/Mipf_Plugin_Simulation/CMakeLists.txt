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

project(Mipf_Plugin_Simulation)
file(GLOB Headers "./*.h") 
file(GLOB UI_Srcs "./*.ui")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
  
#DER
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/DER/libs/cml-1_0_2
                    ${CMAKE_CURRENT_SOURCE_DIR}/DER/libs/dlib-18.9) 
file(GLOB DER_Headers "./DER/*.h")
AUX_SOURCE_DIRECTORY(./DER DER_DIR_SRCS) 
source_group(DER FILES ${DER_DIR_SRCS} ${DER_Headers})
 
#PBD
include_directories(D:/Projects/Third/PositionBasedDynamics
                    ${Eigen_INCLUDE_DIR}
                        ${VMTK_INCLUDE_DIR})
file(GLOB PBD_Headers "./PBD/*.h")
AUX_SOURCE_DIRECTORY(./PBD PBD_DIR_SRCS)
source_group(PBD FILES ${PBD_DIR_SRCS} ${PBD_Headers})

add_definitions(-DUS_MODULE_NAME)

set(CMAKE_AUTOMOC ON)

link_directories(D:/Projects/Third/PositionBasedDynamics/lib/Release)

qt5_wrap_ui(ui_Files ${UI_Srcs})
add_library(Mipf_Plugin_Simulation SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs} ${DER_Headers} ${DER_DIR_SRCS} ${PBD_DIR_SRCS} ${PBD_Headers})
qt5_use_modules(Mipf_Plugin_Simulation Core Gui Widgets)
target_link_libraries(Mipf_Plugin_Simulation    
${VTK_LIBRARIES} ${ITK_LIBRARIES} ${VMTK_LIBRARIES}    
MitkQtWidgets MitkQtWidgetsExt
freeglut.lib
optimized D:/Projects/Third/PositionBasedDynamics/lib/Release/Utils.lib
optimized D:/Projects/Third/PositionBasedDynamics/lib/Release/PositionBasedDynamics.lib 
optimized D:/Projects/Third/PositionBasedDynamics/lib/Release/Simulation.lib 
debug D:/Projects/Third/PositionBasedDynamics/lib/Debug/Utils_d.lib
debug D:/Projects/Third/PositionBasedDynamics/lib/Debug/PositionBasedDynamics_d.lib 
debug D:/Projects/Third/PositionBasedDynamics/lib/Debug/Simulation_d.lib 

)
link_framework()
link_MitkCoreExt()
link_ITKVTKHelpers()

