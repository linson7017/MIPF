cmake_minimum_required(VERSION 2.8.8)
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()
if(POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW) # CMake 3.0
endif()
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW) # CMake 3.0
endif()
if(POLICY CMP0053)
  cmake_policy(SET CMP0053 NEW) # CMake 3.1
endif()

project(WorkBench)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

file (GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)
add_executable(WorkBench ${DIR_SRCS} ${Headers})
qt5_use_modules(WorkBench Widgets)
link_qf()
InstallQfConfig(${PROJECT_NAME})

#Release去控制台窗口
#Set_Target_Properties(WorkBench PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
