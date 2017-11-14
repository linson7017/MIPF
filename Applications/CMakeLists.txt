cmake_minimum_required(VERSION 2.8.8)

FIND_PACKAGE(QTFRAMEWORK REQUIRED)
FIND_PACKAGE(QFMAIN REQUIRED)

macro(InstallQfConfig projectname)
add_custom_command(TARGET ${projectname} POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy_directory
                  ${CMAKE_SOURCE_DIR}/bin/qfconfig/${projectname} ${CMAKE_BINARY_DIR}/bin/qfconfig/${projectname})
endmacro(InstallQfConfig) 

add_subdirectory(WorkBench)
add_subdirectory(PluginGenerator)
add_subdirectory(RobotControlApp)
add_subdirectory(ModuleGenerator)
add_subdirectory(MapperTest)
add_subdirectory(TestApp)

