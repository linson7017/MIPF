
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/Modules/include)
include_directories(${PROJECT_SOURCE_DIR}/Modules/interface)

find_package(QTFRAMEWORK REQUIRED)
find_package(QFMAIN REQUIRED)


add_definitions(-DQF_API_EXPORTS)

add_subdirectory(MitkStdWidgets)

add_subdirectory(ItkAlgorithmSetView)

add_subdirectory(SliceBySliceTrackingView)

add_subdirectory(TubularTrackingView)

add_subdirectory(ImageRegistrationView)
