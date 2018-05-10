
set(MODULES_DIR ${CMAKE_SOURCE_DIR}/Modules)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${MODULES_DIR}/include)
include_directories(${MODULES_DIR}/interface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DQF_API_EXPORTS)

add_subdirectory(MipfPluginBase)

macro(link_framework)
	link_qf()
	link_MipfPluginBase()
endmacro(link_framework)

add_subdirectory(Mipf_Plugin_MitkStdWidgets)

add_subdirectory(Mipf_Plugin_ImageRegistration)

add_subdirectory(Mipf_Plugin_MitkSegmentation)

add_subdirectory(Mipf_Plugin_GraphcutSegmentation)

add_subdirectory(Mipf_Plugin_ImageCropper)

add_subdirectory(Mipf_Plugin_SFLSSegmentation)

add_subdirectory(Mipf_Plugin_Utils)

add_subdirectory(Mipf_Plugin_BasicImageProcess)

add_subdirectory(Mipf_Plugin_Growcut)

add_subdirectory(Mipf_Plugin_AirwaySegmentation)

add_subdirectory(Mipf_Plugin_VesselSegmentation)

if(USE_VMTK)
add_subdirectory(Mipf_Plugin_VMTKTools)
endif()

add_subdirectory(Mipf_Plugin_StaticisTools)

add_subdirectory(Mipf_Plugin_ItkAlgorithmSet)

add_subdirectory(Mipf_Plugin_SurfaceCut)

add_subdirectory(Mipf_Plugin_MapperTest)

add_subdirectory(Mipf_Plugin_ManualSegmentationTest)

add_subdirectory(Mipf_Plugin_VolumeCut)
 
add_subdirectory(Mipf_Plugin_ASM)
 
#add_subdirectory(Mipf_Plugin_Mevislab)
 
if(USE_PCL)
add_subdirectory(Mipf_Plugin_PCL)
endif()
 
add_subdirectory(Mipf_Plugin_ImageBatchProcessing)
 
if(USE_PYTHON)
add_subdirectory(Mipf_Plugin_DeepMedic)
endif()
 
add_subdirectory(Mipf_Plugin_ModelExporter)
 
add_subdirectory(Mipf_Plugin_TubularTracking)

add_subdirectory(Mipf_Plugin_SliceBySliceTracking)
 
add_subdirectory(Mipf_Plugin_SphereCropper)
 
add_subdirectory(Mipf_Plugin_TruncusSegmentation)
 
add_subdirectory(Mipf_Plugin_RSSSegmentation)
 
add_subdirectory(Mipf_Plugin_Simulation)
 
add_subdirectory(Mipf_Plugin_CVA)
 
add_subdirectory(Mipf_Plugin_DSATool)
 
 
add_subdirectory(Mipf_Plugin_Test)
 
add_subdirectory(Mipf_Plugin_Launcher)