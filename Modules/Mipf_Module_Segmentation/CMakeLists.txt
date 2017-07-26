cmake_minimum_required(VERSION 2.8.11)
if (POLICY CMP0020)
   cmake_policy(SET CMP0020 NEW)
endif()
if (POLICY CMP0025)
   cmake_policy(SET CMP0025 NEW) # CMake 3.0
endif()
if (POLICY CMP0043)
   cmake_policy(SET CMP0043 NEW) # CMake 3.0
endif()
if (POLICY CMP0053)
   cmake_policy(SET CMP0053 NEW) # CMake 3.1
endif()

project(Mipf_Module_Segmentation)

include_directories(${ITKVTK_HELPERS_INCLUDE_DIR})

file(GLOB Headers "./*.h")
file(GLOB AIRWAY_Headers "./Airway/*.h")
file(GLOB FASTGROWCUT_Headers "./FastGrowCut/*.h" "*.hxx")
file(GLOB_RECURSE GRAPHCUT_Files "./Graphcut/" "*.h" "*.hpp" "*.hxx" "*.cpp")

AUX_SOURCE_DIRECTORY(./ DIR_SRCS)
AUX_SOURCE_DIRECTORY(./Airway AIRWAY_DIR_SRCS)
AUX_SOURCE_DIRECTORY(./FastGrowCut FASTGROWCUT_DIR_SRCS)



FIND_PACKAGE(PocoFoundation REQUIRED)

add_library(Mipf_Module_Segmentation SHARED 
			${DIR_SRCS} ${AIRWAY_DIR_SRCS} ${FASTGROWCUT_DIR_SRCS} 
			${Headers} ${AIRWAY_Headers} ${FASTGROWCUT_Headers} ${GRAPHCUT_Files}) 
target_link_libraries(Mipf_Module_Segmentation ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} ${ITKVTK_HELPERS_LIBRARIES})

