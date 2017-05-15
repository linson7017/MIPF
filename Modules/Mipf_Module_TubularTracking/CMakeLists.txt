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

project(Mipf_Module_TubularTracking)

file (GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

include_directories(${Boost_INCLUDE_DIR})
FIND_PACKAGE(NEWMAT REQUIRED)


add_library(Mipf_Module_TubularTracking SHARED ${DIR_SRCS} ${Headers} )
target_link_libraries(Mipf_Module_TubularTracking ${VTK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} ${OPENCV_LIBRARIES} ${NEWMAT_LIBRARIES})
