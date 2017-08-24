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

project(Mipf_Module_VesselTools)

file(GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(./ DIR_SRCS)
include_directories(${VMTK_INCLUDE_DIR})
include_directories(${ITKVTK_HELPERS_INCLUDE_DIR})

FIND_PACKAGE(PocoFoundation REQUIRED)

add_library(Mipf_Module_VesselTools SHARED ${DIR_SRCS} ${Headers})
target_link_libraries(Mipf_Module_VesselTools ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${VMTK_LIBRARIES} ${ITKVTK_HELPERS_LIBRARIES} 
${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES}  )

