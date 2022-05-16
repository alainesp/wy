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

option(WY_BUILD_EXAMPLE "Build the example" OFF)
option(WY_BUILD_PERFORMANCE "Build the performance testing" OFF)

###############################################################################################################
# Library configuration
###############################################################################################################
add_library(wy STATIC "wy.cpp")
target_include_directories(wy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(WY_BUILD_EXAMPLE)
    add_executable(wyexamples "example.cpp")
    target_link_libraries(wyexamples PRIVATE wy)
endif()

if(WY_BUILD_PERFORMANCE)
    add_executable(wyperformance "performance.cpp")
    set_property(TARGET wyperformance PROPERTY CXX_STANDARD 20)	 # C++ language to use
    target_link_libraries(wyperformance PRIVATE wy)
endif()
