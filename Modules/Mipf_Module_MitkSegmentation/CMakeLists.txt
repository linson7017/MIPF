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

project(Mipf_Module_MitkSegmentation)

file (GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

FIND_PACKAGE(PocoFoundation REQUIRED)

set(CMAKE_AUTOMOC ON)

#add_definitions(-DUS_MODULE_NAME)

add_library(Mipf_Module_MitkSegmentation SHARED ${DIR_SRCS} ${Headers} )
qt5_use_modules(Mipf_Module_MitkSegmentation Core  Concurrent)
target_link_libraries(Mipf_Module_MitkSegmentation ${QFMAIN_LIBRARIES} ${QFCOMMON_LIBRARIES} MitkCore MitkQtWidgets MitkQtWidgetsExt MitkSegmentation MitkSegmentationUI MitkMultilabel)
