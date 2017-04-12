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

project(NDIPlugin)
file(GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

set(CMAKE_AUTOMOC ON)

FIND_PACKAGE(NEWMAT REQUIRED)

if (VTK_QT_VERSION VERSION_GREATER "4")

	add_library(NDIPlugin SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
	qt5_use_modules(NDIPlugin Core Gui Widgets Network)
	target_link_libraries(NDIPlugin ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt MitkLegacyIO  ${NEWMAT_LIBRARIES})

else()
	add_library(NDIPlugin SHARED ${ DIR_SRCS } ${ Headers } ${ UI_Srcs } ${ MOC_Hdrs })
	target_link_libraries(NDIPlugin ${QT_LIBRARIES} ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES} MitkQtWidgets MitkQtWidgetsExt MitkLegacyIO  ${NEWMAT_LIBRARIES})

endif()
