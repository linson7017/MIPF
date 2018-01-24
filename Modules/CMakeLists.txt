cmake_minimum_required(VERSION 2.8.11)


FIND_PACKAGE(QTFRAMEWORK REQUIRED)
FIND_PACKAGE(QFMAIN REQUIRED)
FIND_PACKAGE(QFCOMMON REQUIRED)

include(${VTK_USE_FILE})
include(${ITK_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(MODULES_DIR ${CMAKE_SOURCE_DIR}/Modules)
include_directories(
	${MODULES_DIR}/include
	${MODULES_DIR}/interface
)

add_definitions(-DQF_API_EXPORTS)

add_subdirectory(ITKVTKHelpers )

add_subdirectory(MitkCoreExt )

add_subdirectory(Mipf_Module_Core)

add_subdirectory(Mipf_Module_MitkMain)

add_subdirectory(Mipf_Module_MitkSegmentation)

add_subdirectory(Mipf_Module_SliceBySliceTracking)

add_subdirectory(Mipf_Module_TubularTracking)

add_subdirectory(Mipf_Module_MathUtil)

add_subdirectory(Mipf_Module_MitkStd)

add_subdirectory(Mipf_Module_Segmentation)

add_subdirectory(Mipf_Module_MitkImageUtils)

if(USE_VMTK)
add_subdirectory(Mipf_Module_VesselTools)
endif()

add_subdirectory(Mipf_Module_Test)
