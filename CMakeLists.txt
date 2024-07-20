###############################################################################################################
# This file is a C++ wrapper around wyhash: 
# https://github.com/wangyi-fudan/wyhash
# 
# Copyright (c) 2022-2024 by Alain Espinosa.
###############################################################################################################
cmake_minimum_required (VERSION 3.24)

project (wy VERSION 1.1.0.0 DESCRIPTION "Fast hashing and pseudo-random number generator"
			HOMEPAGE_URL https://github.com/alainesp/wy LANGUAGES CXX)

# Warning
if(CMAKE_SIZEOF_VOID_P EQUAL 8)# 64 bits
    # No problem
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)# 32 bits
    message(WARNING "32 bits detected -> Non-optimal performance")
endif()

###############################################################################################################
# Library configuration
###############################################################################################################
add_library(wy INTERFACE)
target_include_directories(wy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET wy PROPERTY COMPILE_WARNING_AS_ERROR ON) # Warning level 4 and all warnings as errors

###############################################################################################################
# Executables
###############################################################################################################
option(WY_BUILD_EXAMPLE "Build the example" OFF)
option(WY_BUILD_PERFORMANCE "Build the performance testing" OFF)
option(WY_BUILD_TESTS "Build the tests" OFF)

if(WY_BUILD_EXAMPLE)
    add_executable(wyexamples "example.cpp")
    set_property(TARGET wyexamples PROPERTY CXX_STANDARD 20)	 # C++ language to use
    target_link_libraries(wyexamples PRIVATE wy)
    set_property(TARGET wyexamples PROPERTY COMPILE_WARNING_AS_ERROR ON) # Warning level 4 and all warnings as errors
endif()

include(FetchContent)

if(WY_BUILD_PERFORMANCE)
    add_executable(wyperformance "performance.cpp")
    set_property(TARGET wyperformance PROPERTY CXX_STANDARD 20)	 # C++ language to use

    FetchContent_Declare(benchmark URL https://github.com/google/benchmark/archive/refs/tags/v1.8.5.zip)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
    target_link_libraries(wyperformance PRIVATE wy benchmark::benchmark benchmark::benchmark_main)
    set_property(TARGET wyperformance PROPERTY COMPILE_WARNING_AS_ERROR ON) # Warning level 4 and all warnings as errors
endif()

###############################################################################################################
# Testing
###############################################################################################################
if(WY_BUILD_TESTS)
    SET(BUILD_GMOCK OFF)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.15.0.zip)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(runUnitTests "tests.cpp")
    set_property(TARGET runUnitTests PROPERTY CXX_STANDARD 20) # C++ language to use
    target_link_libraries(runUnitTests PRIVATE wy gtest_main)
     set_property(TARGET runUnitTests PROPERTY COMPILE_WARNING_AS_ERROR ON) # Warning level 4 and all warnings as errors

    include(GoogleTest)
    gtest_discover_tests(runUnitTests)
endif()
