cmake_minimum_required(VERSION 3.9)
project("xfce4-dynamic-opacity")
set(CMAKE_C_STANDARD 11)

add_compile_definitions(PACKAGE_NAME="xfce4-dynamic-opacity")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(source)

#/usr/share/xfce4/panel/plugins

install(FILES opacity-plugin.desktop DESTINATION /usr/share/xfce4/panel/plugins)
install(FILES source/libopacity-plugin.so DESTINATION /usr/lib/xfce4/panel/plugins)
