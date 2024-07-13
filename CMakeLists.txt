cmake_minimum_required(VERSION 3.29.0)

project(type_proofs LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fdiagnostics-color=always)

add_executable(main main.cpp)

add_library(test_prop test_prop.cpp)