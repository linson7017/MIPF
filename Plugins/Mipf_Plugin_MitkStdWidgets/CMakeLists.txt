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


file(GLOB DIR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}  *.h* *.c*)
file(GLOB_RECURSE DataManager_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/DataManagerWidget/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/DataManagerWidget/*.c*)
file(GLOB_RECURSE MultiViews_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/MultiViewsWidget/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/MultiViewsWidget/*.c* )
file(GLOB_RECURSE VolumeVisualization_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/VolumeVisualizationWidget/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/VolumeVisualizationWidget/*.c* )
file(GLOB_RECURSE ImageNavigator_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/ImageNavigatorWidget/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/ImageNavigatorWidget/*.c* )
file(GLOB_RECURSE Measurement_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/MeasurementWidget/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/MeasurementWidget/*.c* )
file(GLOB_RECURSE Status_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/StatusWidget/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/StatusWidget/*.c* )
file(GLOB_RECURSE RenderWindow_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/RenderWindow/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/RenderWindow/*.c* )
file(GLOB_RECURSE Properties_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/Properties/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/Properties/*.c* )
file(GLOB_RECURSE Properties_UI_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/Properties/*.ui)

set(UI_Srcs ${Properties_UI_SRCS})

source_group(DataManager FILES  ${DataManager_DIR_SRCS})
source_group(MultiViews FILES  ${MultiViews_DIR_SRCS})
source_group(VolumeVisulization FILES  ${VolumeVisualization_DIR_SRCS})
source_group(ImageNavigator FILES  ${ImageNavigator_DIR_SRCS})
source_group(Measurement FILES  ${Measurement_DIR_SRCS})
source_group(Status FILES  ${Status_DIR_SRCS})
source_group(RenderWindow FILES  ${RenderWindow_DIR_SRCS})
source_group(Properties FILES  ${Properties_DIR_SRCS} ${Properties_UI_SRCS})

set(CMAKE_AUTOMOC ON)

include_directories()

#使用mitk的初始化模块
add_definitions(-DUS_MODULE_NAME)

qt5_wrap_ui(ui_Files ${UI_Srcs})
add_library(Mipf_Plugin_MitkStdWidgets SHARED 
	${DIR_SRCS} 
	${DataManager_DIR_SRCS} 
	${MultiViews_DIR_SRCS} 
	${VolumeVisualization_DIR_SRCS} 
	${ImageNavigator_DIR_SRCS} 
	${Measurement_DIR_SRCS} 
	${Status_DIR_SRCS}
	${RenderWindow_DIR_SRCS}
	${Properties_DIR_SRCS}
	${UI_Srcs} 
	${MOC_Hdrs})
qt5_use_modules(Mipf_Plugin_MitkStdWidgets Core Gui Widgets)
target_link_libraries(Mipf_Plugin_MitkStdWidgets ${VTK_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt )
link_framework()
