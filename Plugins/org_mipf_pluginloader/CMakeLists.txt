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

project(org_mipf_pluginloader)


FIND_PACKAGE(CTK REQUIRED)
include_directories(
	${CTKCore_INCLUDE_DIRS}
	${CTKPluginFramework_INCLUDE_DIRS}
)


file (GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

set(CMAKE_AUTOMOC ON)

if(VTK_QT_VERSION VERSION_GREATER "4")

add_library(org_mipf_pluginloader SHARED  ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
qt5_use_modules(org_mipf_pluginloader Core Gui Widgets)
target_link_libraries(org_mipf_pluginloader CTKCore CTKPluginFramework )

else()

add_library(org_mipf_pluginloader SHARED  ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
target_link_libraries(org_mipf_pluginloader CTKCore CTKPluginFramework )

endif()