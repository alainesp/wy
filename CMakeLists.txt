###############################################################################################################
# This file is a C++ wrapper around wyhash: 
# https://github.com/wangyi-fudan/wyhash
# 
# Copyright (c) 2022 by Alain Espinosa.
###############################################################################################################
cmake_minimum_required (VERSION 3.12)

project (wy VERSION 1.0.0.0 DESCRIPTION "Fast hashing and pseudo-random number generator"
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
option(WY_BUILD_EXAMPLE "Build the example" OFF)
option(WY_BUILD_PERFORMANCE "Build the performance testing" OFF)
option(WY_BUILD_TESTS "Build the tests" OFF)

add_library(wy INTERFACE)
target_include_directories(wy INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

###############################################################################################################
# Executables
###############################################################################################################
if(WY_BUILD_EXAMPLE)
    add_executable(wyexamples "example.cpp")
    set_property(TARGET wyexamples PROPERTY CXX_STANDARD 20)	 # C++ language to use
    target_link_libraries(wyexamples PRIVATE wy)

# Warning level 4 and all warnings as errors
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    cmake_policy(SET CMP0135 NEW) # Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24
    set_property(TARGET wyexamples PROPERTY COMPILE_WARNING_AS_ERROR ON)
else()
    if(MSVC)
        target_compile_options(wyexamples PRIVATE /W4 /WX)
        target_link_options(wyexamples PRIVATE /W4 /WX)
    else()
        target_compile_options(wyexamples PRIVATE -Wall -Wextra -pedantic -Werror -Wl,--fatal-warnings)
    endif()
endif()

endif()

if(WY_BUILD_PERFORMANCE)
    add_executable(wyperformance "performance.cpp")
    set_property(TARGET wyperformance PROPERTY CXX_STANDARD 20)	 # C++ language to use
    target_link_libraries(wyperformance PRIVATE wy)

# Warning level 4 and all warnings as errors
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    cmake_policy(SET CMP0135 NEW) # Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24
    set_property(TARGET wyperformance PROPERTY COMPILE_WARNING_AS_ERROR ON)
else()
    if(MSVC)
        target_compile_options(wyperformance PRIVATE /W4 /WX)
        target_link_options(wyperformance PRIVATE /W4 /WX)
    else()
        target_compile_options(wyperformance PRIVATE -Wall -Wextra -pedantic -Werror -Wl,--fatal-warnings)
    endif()
endif()

endif()

###############################################################################################################
# Testing
###############################################################################################################
if(WY_BUILD_TESTS)
    include(FetchContent)
    SET(BUILD_GMOCK OFF)
    FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(runUnitTests "tests.cpp")
    set_property(TARGET runUnitTests PROPERTY CXX_STANDARD 20) # C++ language to use
    target_link_libraries(runUnitTests PRIVATE wy gtest_main)

# Warning level 4 and all warnings as errors
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    cmake_policy(SET CMP0135 NEW) # Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24
    set_property(TARGET runUnitTests PROPERTY COMPILE_WARNING_AS_ERROR ON)
else()
    if(MSVC)
        target_compile_options(runUnitTests PRIVATE /W4 /WX)
        target_link_options(runUnitTests PRIVATE /W4 /WX)
    else()
        target_compile_options(runUnitTests PRIVATE -Wall -Wextra -pedantic -Werror -Wl,--fatal-warnings)
    endif()
endif()

    include(GoogleTest)
    gtest_discover_tests(runUnitTests)
endif()
