cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

# Change project and application name to your own
set(MIPF_PROJECT_NAME MIPFProject)

#-----------------------------------------------------------------------------
# Set a default build type if none was specified
#-----------------------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
               STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


#-----------------------------------------------------------------------------
# Check miminum compiler versions
#-----------------------------------------------------------------------------

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # require at least gcc 4.9 as provided by ppa:ubuntu-toolchain-r/test for Ubuntu 12.04
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    message(FATAL_ERROR "GCC version must be at least 4.9
If you are using Ubuntu 12.04, you can easily install gcc and g++ 4.9 (or any later version available) in addition to your version ${CMAKE_CXX_COMPILER_VERSION}:
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo apt-get update
  sudo apt-get install gcc-4.9 g++-4.9
Make sure to explicitly specify these compilers when configuring MITK:
  CMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc-4.9
  CMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++-4.9
For more information on the proposed PPA see the Toolchain Updates section of https://wiki.ubuntu.com/ToolChain.")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # require at least clang 3.4 as provided by Ubuntu 12.04
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.4)
    message(FATAL_ERROR "Clang version must be at least 3.4")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  # require at least clang 5.0
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    message(FATAL_ERROR "Apple Clang version must be at least 5.0")
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # require at least Visual Studio 2013
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0.40629.0)
    message(FATAL_ERROR "Microsoft Visual Studio 2013 Update 5 or newer required (MSVC 18.0.40629.0)")
  endif()
else()
  message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang (Linux or Apple), GCC and MSVC.")
endif()

set(CMAKE_CXX_EXTENSIONS 0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 1)

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

set(project_policies
  CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  CMP0007 # NEW: List command no longer ignores empty elements.
  CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  CMP0010 # NEW: Bad variable reference syntax is an error.
  CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  CMP0012 # NEW: if() recognizes numbers and boolean constants.
  CMP0013 # NEW: Duplicate binary directories are not allowed.
  CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
foreach(policy ${project_policies})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()

#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
# Output directories.
#-----------------------------------------------------------------------------

#foreach(type LIBRARY RUNTIME ARCHIVE)
  # set(output_dir ${PROJECT_BINARY_DIR}/obj)
  # set(CMAKE_${type}_OUTPUT_DIRECTORY ${output_dir} CACHE INTERNAL "Single output directory for building all libraries.")
  # mark_as_advanced(CMAKE_${type}_OUTPUT_DIRECTORY)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
	mark_as_advanced(CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
	mark_as_advanced(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	mark_as_advanced(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	message(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
	message(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
	message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#endforeach()



#-----------------------------------------------------------------------------
# Prerequesites
#-----------------------------------------------------------------------------

find_package(MITK 2016.03.0 REQUIRED)

if(COMMAND mitkFunctionCheckMitkCompatibility)
  mitkFunctionCheckMitkCompatibility(VERSIONS MITK_VERSION_PLUGIN_SYSTEM 1 REQUIRED)
else()
  message(SEND_ERROR "Your MITK version is too old. Please use Git hash b86bf28 or newer")
endif()

link_directories(${MITK_LINK_DIRECTORIES})

set(${PROJECT_NAME}_VERSION_MAJOR "0")
set(${PROJECT_NAME}_VERSION_MINOR "1")
set(${PROJECT_NAME}_VERSION_PATCH "1")
set(${PROJECT_NAME}_VERSION_STRING "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")

# Look for optional Doxygen package
find_package(Doxygen)

# Ask the user if a console window should be shown with the applications
option(${PROJECT_NAME}_SHOW_CONSOLE_WINDOW "Use this to enable or disable the console window when starting GUI Applications" ON)
mark_as_advanced(${PROJECT_NAME}_SHOW_CONSOLE_WINDOW)

#-----------------------------------------------------------------------------
# Set symbol visibility Flags
#-----------------------------------------------------------------------------

# MinGW does not export all symbols automatically, so no need to set flags
if(CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
  # The MITK module build system does not yet support default hidden visibility
  set(VISIBILITY_CXX_FLAGS ) # "-fvisibility=hidden -fvisibility-inlines-hidden")
endif()

#-----------------------------------------------------------------------------
# Set coverage Flags
#-----------------------------------------------------------------------------

if(WITH_COVERAGE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(coverage_flags "-g -fprofile-arcs -ftest-coverage -O0 -DNDEBUG")
    set(COVERAGE_CXX_FLAGS ${coverage_flags})
    set(COVERAGE_C_FLAGS ${coverage_flags})
  endif()
endif()

#-----------------------------------------------------------------------------
# Set C/CXX Flags
#-----------------------------------------------------------------------------

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${${PROJECT_NAME}_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${${PROJECT_NAME}_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${${PROJECT_NAME}_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${${PROJECT_NAME}_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${${PROJECT_NAME}_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${${PROJECT_NAME}_CXX_FLAGS_RELEASE}")

set(CMAKE_EXE_LINKER_FLAGS ${${PROJECT_NAME}_EXE_LINKER_FLAGS})
set(CMAKE_SHARED_LINKER_FLAGS ${${PROJECT_NAME}_SHARED_LINKER_FLAGS})
set(CMAKE_MODULE_LINKER_FLAGS ${${PROJECT_NAME}_MODULE_LINKER_FLAGS})

#-----------------------------------------------------------------------------
# ${MIPF_PROJECT_NAME}_SUPERBUILD_BINARY_DIR
#-----------------------------------------------------------------------------

# If ${MIPF_PROJECT_NAME}_SUPERBUILD_BINARY_DIR isn't defined, it means this project is
# *NOT* build using Superbuild. In that specific case, ${MIPF_PROJECT_NAME}_SUPERBUILD_BINARY_DIR
# should default to PROJECT_BINARY_DIR
if(NOT DEFINED ${PROJECT_NAME}_SUPERBUILD_BINARY_DIR)
  set(${PROJECT_NAME}_SUPERBUILD_BINARY_DIR ${PROJECT_BINARY_DIR})
endif()

#-----------------------------------------------------------------------------
# Qt support
#-----------------------------------------------------------------------------

if(MITK_USE_QT)
  set(QT_QMAKE_EXECUTABLE ${MITK_QMAKE_EXECUTABLE})
  add_definitions(-DQWT_DLL)
endif()


#Use External Libraries
if(NOT USE_VMTK)
	set(USE_VMTK ON CACHE BOOL "use vmtk")
endif()

if(NOT USE_PCL)
	set(USE_PCL ON CACHE BOOL "use pcl plugin")
endif()

if(NOT USE_PYTHON)
	set(USE_PYTHON ON CACHE BOOL "use python plugin")
endif()
if(USE_PYTHON)
	find_package(PythonLibs REQUIRED)
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#add framework dependencies
FIND_PACKAGE(QTXML REQUIRED)
FIND_PACKAGE(QFMAIN REQUIRED)
macro(link_qf)
	target_link_libraries(${PROJECT_NAME} ${QTXML_LIBRARIES} ${QFMAIN_LIBRARIES})
endmacro(link_qf)

set(QF_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/qfconfig)

#-----------------------------------------------------------------------------
# Add modules
#-----------------------------------------------------------------------------

add_subdirectory(Modules)
#-----------------------------------------------------------------------------
# Add plugins
#-----------------------------------------------------------------------------

add_subdirectory(Plugins)

#-----------------------------------------------------------------------------
# Add subdirectories
#-----------------------------------------------------------------------------
add_subdirectory(Applications)

if(WIN32)
  include(mitkFunctionCreateWindowsBatchScript)

  set(VS_SOLUTION_FILE "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.sln")
  foreach(VS_BUILD_TYPE debug release)
    mitkFunctionCreateWindowsBatchScript("${PROJECT_SOURCE_DIR}/CMake/StartVS.bat.in"
      ${PROJECT_BINARY_DIR}/StartVS_${VS_BUILD_TYPE}.bat
      ${VS_BUILD_TYPE})
  endforeach()
endif(WIN32)



