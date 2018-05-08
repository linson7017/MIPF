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

project(MipfPluginBase)

file(GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(./ DIR_SRCS) 

set(MipfPluginBase_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/"  CACHE INTERNAL "MipfPluginBase headers")
set(MipfPluginBase_LIBRARIES 
	optimized ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Release/MipfPluginBase.lib debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Debug/MipfPluginBase.lib  
	CACHE INTERNAL "MipfPluginBase library" )

macro(link_MipfPluginBase)
	include_directories(${MipfPluginBase_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${MipfPluginBase_LIBRARIES})
endmacro(link_MipfPluginBase) 

add_library(MipfPluginBase SHARED ${DIR_SRCS} ${Headers})
target_link_libraries(MipfPluginBase MitkQtWidgets)
link_qf()

