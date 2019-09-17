cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 11)

project(source)

# Create libs
add_library(opacity opacity.c)
add_library(opacity-dialogs opacity-dialogs.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(common)
