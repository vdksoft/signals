
cmake_minimum_required(VERSION 3.7)

project(vdk-signals CXX)

# Define two libraries, one for the normal version, one for the lite version.
add_library(vdk-signals INTERFACE)
target_sources(vdk-signals INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vdk_signals/signals.h)
target_include_directories(vdk-signals INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(vdk-signals-lite INTERFACE)
target_sources(vdk-signals-lite INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vdk_signals/signals.h)
target_include_directories(vdk-signals-lite INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(vdk-signals-lite INTERFACE VDK_SIGNALS_LITE)

option(VDK_SIGNALS_TEST_ENABLED "Enable VDK signals unit testing" OFF)
if(VDK_SIGNALS_TEST_ENABLED)
    add_subdirectory(tests)
endif()
