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


file(GLOB SRCS ${CMAKE_CURRENT_SOURCE_DIR} *.h* *.c*)
file(GLOB_RECURSE AIRWAY_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/Airway/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/Airway/*.c*)
file(GLOB_RECURSE FASTGROWCUT_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/FastGrowCut/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/FastGrowCut/*.c* )
file(GLOB_RECURSE GRAPHCUT_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/Graphcut/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/Graphcut/*.c* )
file(GLOB_RECURSE RSS_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/RSS/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/RSS/*.c* )

source_group(Airway FILES  ${AIRWAY_SRCS})
source_group(FastGrowcut FILES  ${FASTGROWCUT_SRCS})
source_group(Graphcut FILES  ${GRAPHCUT_SRCS})
source_group(RSS FILES  ${RSS_SRCS})

add_library(Mipf_Module_Segmentation SHARED 
			${SRCS} ${AIRWAY_SRCS} ${FASTGROWCUT_SRCS} ${GRAPHCUT_SRCS} ${RSS_SRCS}) 
			
target_link_libraries(Mipf_Module_Segmentation ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} ${ITKVTK_HELPERS_LIBRARIES})

