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

project(ITKVTKHelpers)

set(ITKVTK_HELPERS_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/"  CACHE INTERNAL "ITKVTKHelpers headers" )

set(ITKVTK_HELPERS_LIBRARIES 
	optimized ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Release/ITKVTKHelpers.lib debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Debug/ITKVTKHelpers.lib  
	CACHE INTERNAL "ITKVTKHelpers library" )
	
macro(link_ITKVTKHelpers)
	include_directories(${ITKVTK_HELPERS_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${ITKVTK_HELPERS_LIBRARIES})
endmacro(link_ITKVTKHelpers)

file(GLOB SRCS "./*.h" "*.hpp" "*.cpp")

add_library(ITKVTKHelpers SHARED ${SRCS}) 

target_link_libraries(ITKVTKHelpers ${VTK_LIBRARIES} ${ITK_LIBRARIES})

