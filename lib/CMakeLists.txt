# simpleConfig/lib

add_library(simpleConfig
    simpleConfig.cpp
    value_type.cpp
    validator.cpp
)

target_include_directories(simpleConfig PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
