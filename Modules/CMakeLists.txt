cmake_minimum_required(VERSION 2.8.11)


FIND_PACKAGE(QTFRAMEWORK REQUIRED)
FIND_PACKAGE(QFMAIN REQUIRED)

include(${VTK_USE_FILE})
include(${ITK_USE_FILE})

set(MODULES_DIR ${CMAKE_SOURCE_DIR}/Modules)
include_directories(
	${MODULES_DIR}/include
	${MODULES_DIR}/interface
)

add_definitions(-DQF_API_EXPORTS)

add_subdirectory(MitkMain)

add_subdirectory(SliceBySliceTracking)

add_subdirectory(TubularTracking)

add_subdirectory(MathUtil)