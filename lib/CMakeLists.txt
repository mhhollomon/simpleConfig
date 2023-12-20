# simpleConfig/lib

add_library(simpleConfig simple_config.cpp)

target_include_directories(simpleConfig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
