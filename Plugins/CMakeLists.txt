
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/Modules/include)
include_directories(${PROJECT_SOURCE_DIR}/Modules/interface)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QTFRAMEWORK REQUIRED)
find_package(QFMAIN REQUIRED)

add_definitions(-DQF_API_EXPORTS)

add_subdirectory(Mipf_Plugin_MitkStdWidgets)

add_subdirectory(Mipf_Plugin_ItkAlgorithmSet)

add_subdirectory(Mipf_Plugin_SliceBySliceTracking)

add_subdirectory(Mipf_Plugin_TubularTracking)

add_subdirectory(Mipf_Plugin_ImageRegistration)

add_subdirectory(Mipf_Plugin_MitkSegmentation)