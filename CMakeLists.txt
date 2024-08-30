cmake_minimum_required(VERSION 3.22.0)

project(type_proofs LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-fdiagnostics-color=always)

add_library(test_prop EXCLUDE_FROM_ALL test_prop.cpp)
add_library(test_algo_positive EXCLUDE_FROM_ALL subst_test/positive.cpp)
add_library(test_algo_negative_control EXCLUDE_FROM_ALL subst_test/negative_control.cpp)
add_library(test_bad_bound_subst1 EXCLUDE_FROM_ALL subst_test/negative/bound_subst1.cpp)
add_library(test_bad_bound_subst2 EXCLUDE_FROM_ALL subst_test/negative/bound_subst2.cpp)
add_library(tag_test EXCLUDE_FROM_ALL tag_test.cpp)
add_library(demo_bad_usage EXCLUDE_FROM_ALL demo_bad_usage.cpp)


enable_testing()

add_test(NAME test_algo_positive
    COMMAND cmake --build . --target test_algo_positive
)
add_test(NAME test_algo_negative_control
    COMMAND cmake --build . --target test_algo_negative_control
)
add_test(NAME test_proofs
    COMMAND cmake --build . --target test_prop
)
add_test(NAME test_bad_bound_subst1
    COMMAND cmake --build . --target test_bad_bound_subst1
)
set_property(TEST test_bad_bound_subst1 PROPERTY WILL_FAIL TRUE)
add_test(NAME test_bad_bound_subst2
    COMMAND cmake --build . -- test_bad_bound_subst2
)
set_property(TEST test_bad_bound_subst2 PROPERTY WILL_FAIL TRUE)
add_test(NAME tag_test
    COMMAND cmake --build . --target tag_test
)
add_test(NAME demo_bad_usage
    COMMAND cmake --build . --target demo_bad_usage
)