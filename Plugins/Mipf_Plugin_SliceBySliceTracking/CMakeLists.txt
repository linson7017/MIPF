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

project(Mipf_Plugin_SliceBySliceTracking)

file (GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

set(CMAKE_AUTOMOC ON)

if(VTK_QT_VERSION VERSION_GREATER "4")

add_library(Mipf_Plugin_SliceBySliceTracking SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
qt5_use_modules(Mipf_Plugin_SliceBySliceTracking Core Gui Widgets)
target_link_libraries(Mipf_Plugin_SliceBySliceTracking ${VTK_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt)
link_framework()
else()

add_library(Mipf_Plugin_SliceBySliceTracking SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
target_link_libraries(Mipf_Plugin_SliceBySliceTracking ${QT_LIBRARIES} ${VTK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt)

endif()