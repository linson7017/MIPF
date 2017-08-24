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

project(TestApp)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/Modules/interface
)

file (GLOB Headers "./*.h")

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)


if(VTK_QT_VERSION VERSION_GREATER "4")
  find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)
  qt5_wrap_ui(UI_Srcs ${UIs})

  # CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
  add_executable(TestApp ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
  qt5_use_modules(TestApp Core Gui Widgets)
  target_link_libraries(TestApp  ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt MitkSegmentation)
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
  qt4_wrap_ui(UI_Srcs ${UIs})
  add_executable(TestApp ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
  target_link_libraries(TestApp ${QT_LIBRARIES}  ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES}) 
endif()

#Release去控制台窗口
#Set_Target_Properties(WorkBench PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

# add_custom_command(TARGET WorkBench
  # POST_BUILD 
  # COMMAND copy "$(OutDir)/$(TargetName).exe" "../qf/bin"
  # COMMENT "copy to qf bin"
 # )