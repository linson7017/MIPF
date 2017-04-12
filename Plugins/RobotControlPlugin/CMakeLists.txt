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

project(RobotControlPlugin)
file(GLOB Headers "./*.h")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

include_directories(
	D:/robot-soft/trunk/Code/include
)

link_directories(
	D:/robot-soft/trunk/Code/lib/release
)

set(CMAKE_AUTOMOC ON)

if (VTK_QT_VERSION VERSION_GREATER "4")

    add_library(RobotControlPlugin SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
    qt5_use_modules(RobotControlPlugin Core Gui Widgets)
    target_link_libraries(RobotControlPlugin   ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES}  hardwareController.lib utility.lib)

else()
    add_library(RobotControlPlugin SHARED ${DIR_SRCS} ${Headers} ${UI_Srcs} ${MOC_Hdrs})
    target_link_libraries(RobotControlPlugin ${QT_LIBRARIES}   ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES}  hardwareController.lib utility.lib)

endif()
