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

project(PluginGenerator)


include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

file (GLOB Headers "./*.h")

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

if(VTK_QT_VERSION VERSION_GREATER "4")
  find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)
  qt5_wrap_ui(UI_Srcs ${UIs})

  # CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
  add_executable(PluginGenerator ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
  qt5_use_modules(PluginGenerator Core Gui Widgets)
  link_framework()
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  qt4_wrap_ui(UI_Srcs ${UIs})

  add_executable(PluginGenerator ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
  target_link_libraries(PluginGenerator ${QT_LIBRARIES})
  link_framework()
endif()

InstallQfConfig(${PROJECT_NAME})

 
 

