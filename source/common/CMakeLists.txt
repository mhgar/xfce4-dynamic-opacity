cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 11)

project(common)

# Create libs
add_library(panel-debug panel-debug.c)
add_library(panel-utils panel-utils.c)
add_library(panel-xfconf panel-xfconf.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
