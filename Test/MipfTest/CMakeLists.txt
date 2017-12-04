cmake_minimum_required(VERSION 2.8.8)
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

project(TestApp)

#Set the mipf interface directory. For instance in .../Mipf/Modules/interface
if(NOT MIPF_INTERFACE_DIR)
	set(MIPF_INTERFACE_DIR "MIPF_INTERFACE_DIR-NOTFOUND" CACHE PATH "Mipf interface directory")
    message(SEND_ERROR "Unable to find the mipf interface directory")
endif()

#Find the depend package
find_package(QTFRAMEWORK REQUIRED)
find_package(QFMAIN REQUIRED)
find_package(MITK 2016.03.0 REQUIRED)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MIPF_INTERFACE_DIR}
)

file (GLOB Headers "./*.h")

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# Instruct CMake to run moc automatically when needed.
# CMAKE_AUTOMOC in ON so the MocHdrs will be automatically wrapped.
set(CMAKE_AUTOMOC ON)


find_package(Qt5 COMPONENTS Widgets REQUIRED QUIET)
qt5_wrap_ui(UI_Srcs ${UIs})

add_executable(TestApp ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
qt5_use_modules(TestApp Core Gui Widgets)
target_link_libraries(TestApp  ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkCore)

