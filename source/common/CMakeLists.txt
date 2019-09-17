cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 11)

project(common)

# Create libs
add_library(common panel-debug.c panel-utils.c panel-xfconf.c)

install(FILES panel-private.h panel-debug.h panel-utils.h panel-dbus.h panel-xfconf.h DESTINATION include)

# include_directories(${CMAKE_CURRENT_SOURCE_DIR})
