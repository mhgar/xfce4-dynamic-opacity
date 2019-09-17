cmake_minimum_required(VERSION 3.9)
project("xfce4-dynamic-opacity")
set(CMAKE_C_STANDARD 11)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(source)

# Create libs
add_library(opacity, SHARED, opacity.c)
add_library(opacity-dialogs, SHARED, opacity-dialogs.c)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
