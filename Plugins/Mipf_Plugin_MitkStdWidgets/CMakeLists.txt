cmake_minimum_required(VERSION 2.8.11)
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

project(Mipf_Plugin_MitkStdWidgets)


include_directories(".")

file (GLOB Headers "./*.h")
file (GLOB DataManagerHeaders "./DataManagerWidget/*.h")
file (GLOB MultiViewsHeaders "./MultiViewsWidget/*.h")
file (GLOB VolumeVisualizationHeaders "./VolumeVisualizationWidget/*.h")
file (GLOB ImageNavigatorHeaders "./ImageNavigatorWidget/*.h")
file (GLOB MeasurementHeaders "./MeasurementWidget/*.h")

AUX_SOURCE_DIRECTORY(. DIR_SRCS)
AUX_SOURCE_DIRECTORY(./DataManagerWidget DataManager_DIR_SRCS)
AUX_SOURCE_DIRECTORY(./MultiViewsWidget MultiViews_DIR_SRCS)
AUX_SOURCE_DIRECTORY(./VolumeVisualizationWidget VolumeVisualization_DIR_SRCS)
AUX_SOURCE_DIRECTORY(./ImageNavigatorWidget ImageNavigator_DIR_SRCS)
AUX_SOURCE_DIRECTORY(./MeasurementWidget Measurement_DIR_SRCS)

set(CMAKE_AUTOMOC ON)

include_directories()

#使用mitk的初始化模块
add_definitions(-DUS_MODULE_NAME)

if(VTK_QT_VERSION VERSION_GREATER "4")

add_library(Mipf_Plugin_MitkStdWidgets SHARED 
	${DIR_SRCS} ${DataManager_DIR_SRCS} ${MultiViews_DIR_SRCS} ${VolumeVisualization_DIR_SRCS} ${ImageNavigator_DIR_SRCS} ${Measurement_DIR_SRCS}
	${Headers} ${DataManagerHeaders} ${MultiViewsHeaders} ${VolumeVisualizationHeaders} ${ImageNavigatorHeaders} ${MeasurementHeaders}
	${UI_Srcs} 
	${MOC_Hdrs})
qt5_use_modules(Mipf_Plugin_MitkStdWidgets Core Gui Widgets)
target_link_libraries(Mipf_Plugin_MitkStdWidgets ${VTK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt MitkQtWidgetsWeb)

else()

add_library(Mipf_Plugin_MitkStdWidgets SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
target_link_libraries(Mipf_Plugin_MitkStdWidgets ${QT_LIBRARIES} ${VTK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt MitkQtWidgetsWeb)

endif()