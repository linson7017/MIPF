cmake_minimum_required(VERSION 2.8.8)

FIND_PACKAGE(QTFRAMEWORK REQUIRED)
FIND_PACKAGE(QFMAIN REQUIRED)


add_subdirectory(WorkBench)
add_subdirectory(PluginGenerator)
add_subdirectory(RobotControlApp)
add_subdirectory(ModuleGenerator)
add_subdirectory(MapperTest)
add_subdirectory(TestApp)

