cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(mersenne_twister_engine_test)
find_package(Catch2 3 REQUIRED)
add_executable(tests
        config.h
        constant_values.cpp
        copy.cpp
        equality.cpp
        initial_sequence.cpp
)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

add_subdirectory(.. freestanding_mersenne_twister EXCLUDE_FROM_ALL)
target_link_libraries(tests PRIVATE freestanding_mersenne_twister)
