# Configinator5000/tests

cmake_minimum_required(VERSION 3.13)

## first Test ###########################
set( testname t01-settings)
add_executable (${testname})
target_sources(${testname} PRIVATE "${testname}.cpp")
target_link_libraries(${testname}
    PRIVATE doctest simpleConfig)

add_test(NAME ${testname} COMMAND ${testname})

## Parse Test ###########################
set( Testname t02-parse)
add_executable (${Testname})
target_sources(${Testname} PRIVATE "${Testname}.cpp")
target_link_libraries(${Testname}
    PRIVATE doctest simpleConfig)

add_test(NAME ${Testname} COMMAND ${Testname})

## Parse Schema Test ###########################
set( Testname t03-parse-schema)
add_executable (${Testname})
target_sources(${Testname} PRIVATE "${Testname}.cpp")
target_link_libraries(${Testname}
    PRIVATE doctest simpleConfig)

add_test(NAME ${Testname} COMMAND ${Testname})

## Validate against Schema Test ###########################
set( Testname t04-validate-with-schema)
add_executable (${Testname})
target_sources(${Testname} PRIVATE "${Testname}.cpp")
target_link_libraries(${Testname}
    PRIVATE doctest simpleConfig)

add_test(NAME ${Testname} COMMAND ${Testname})
