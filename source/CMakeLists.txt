cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 11)

project(opacity-plugin C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(XFCE4UI REQUIRED libxfce4ui-2)
pkg_check_modules(XFCE4PANEL REQUIRED libxfce4panel-2.0)
pkg_check_modules(XFCE4UTIL REQUIRED libxfce4util-1.0)
pkg_check_modules(XFCONF REQUIRED libxfconf-0)
pkg_check_modules(WNCK REQUIRED libwnck-3.0)

include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${XFCE4UI_INCLUDE_DIRS})
include_directories(${XFCE4PANEL_INCLUDE_DIRS})
include_directories(${XFCE4UTIL_INCLUDE_DIRS})
include_directories(${XFCONF_INCLUDE_DIRS})
include_directories(${WNCK_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}) # So we can include common

link_directories(${GLIB_LIBRARY_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
link_directories(${XFCE4UI_LIBRARY_DIRS})
link_directories(${XFCE4PANEL_LIBRARY_DIRS})
link_directories(${XFCE4UTIL_LIBRARY_DIRS})
link_directories(${XFCONF_LIBRARY_DIRS})
link_directories(${WNCK_LIBRARY_DIRS})

add_library(opacity-plugin SHARED opacity.c opacity-dialogs.c)

target_link_libraries(opacity-plugin common)
target_link_libraries(opacity-plugin ${GLIB_LIBRARIES})
target_link_libraries(opacity-plugin ${GTK3_LIBRARIES})
target_link_libraries(opacity-plugin ${XFCE4UI_LIBRARY_DIRS})
target_link_libraries(opacity-plugin ${XFCE4PANEL_LIBRARY_DIRS})
target_link_libraries(opacity-plugin ${XFCE4UTIL_LIBRARY_DIRS})
target_link_libraries(opacity-plugin ${XFCONF_LIBRARY_DIRS})
target_link_libraries(opacity-plugin ${WNCK_LIBRARY_DIRS})

add_subdirectory(common)
