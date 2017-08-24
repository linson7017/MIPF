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


file(GLOB DIR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}  *.h* *.c*)
file(GLOB_RECURSE RENDERING_DIR_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.h* 
${CMAKE_CURRENT_SOURCE_DIR}/Rendering/*.c*)
source_group(Rendering FILES  ${RENDERING_DIR_SRCS})

add_definitions(-DUS_MODULE_NAME)

add_library(MitkCoreExt SHARED ${DIR_SRCS}
${RENDERING_DIR_SRCS} )
target_link_libraries(MitkCoreExt ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkCore)
