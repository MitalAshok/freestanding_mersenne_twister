cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(freestanding_mersenne_twister)
add_library(freestanding_mersenne_twister INTERFACE "include/freestanding_mersenne_twister.h")
target_include_directories(freestanding_mersenne_twister INTERFACE "include")
