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

project(MitkCoreExt)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB DIR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}  *.h* *.c*)
 
###Rendering###
file(GLOB_RECURSE RENDERING_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.c*)
source_group(Rendering FILES  ${RENDERING_DIR_SRCS})

###Interactions###
file(GLOB_RECURSE INTERACTIONS_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/Interactions/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/Interactions/*.c*)
source_group(Interactions FILES  ${INTERACTIONS_DIR_SRCS})

###DataTypeExt###
file(GLOB_RECURSE DataTypeExt_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/DataTypeExt/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/DataTypeExt/*.c*)
source_group(DataTypeExt FILES  ${DataTypeExt_DIR_SRCS})

set(MitkCoreExt_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/"  CACHE INTERNAL "MitkCoreExt headers")

set(MitkCoreExt_LIBRARIES 
	optimized ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Release/MitkCoreExt.lib debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Debug/MitkCoreExt.lib  
	CACHE INTERNAL "MitkCoreExt library" )

macro(link_MitkCoreExt)
	include_directories(${MitkCoreExt_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${MitkCoreExt_LIBRARIES})
endmacro(link_MitkCoreExt)

add_definitions(-DUS_MODULE_NAME)

add_library(MitkCoreExt SHARED ${DIR_SRCS}
${RENDERING_DIR_SRCS} ${INTERACTIONS_DIR_SRCS} ${DataTypeExt_DIR_SRCS})
target_link_libraries(MitkCoreExt ${VTK_LIBRARIES} ${ITK_LIBRARIES}
${QFMAIN_LIBRARIES} 
MitkCore MitkDataTypesExt
)
