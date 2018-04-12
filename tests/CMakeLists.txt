
cmake_minimum_required(VERSION 3.7)

project(vdk-signals-test CXX)

enable_testing()
add_subdirectory(googletest)

config_compiler_and_linker() # some gtest stuff

# Build the same tests twice, once for the normal version and once for the lite version.
# The normal version might have more test in other files if need be, for example to test
# thread safety specific functionalities.

set(normal_tests_target vdk-signals-tests)
add_executable(${normal_tests_target} tests.cpp)
target_link_libraries(${normal_tests_target} vdk-signals gmock_main)
target_compile_options(${normal_tests_target} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    $<$<CXX_COMPILER_ID:Clang>:-Wall>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
add_test(NAME ${normal_tests_target} COMMAND ${normal_tests_target})

set(lite_tests_target vdk-signals-lite-tests)
add_executable(${lite_tests_target} tests.cpp)
target_link_libraries(${lite_tests_target} vdk-signals-lite gmock_main)
target_compile_options(${lite_tests_target} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    $<$<CXX_COMPILER_ID:Clang>:-Wall>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
add_test(NAME ${lite_tests_target} COMMAND ${lite_tests_target})
