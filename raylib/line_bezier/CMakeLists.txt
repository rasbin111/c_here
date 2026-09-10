cmake_minimum_required(VERSION 3.10)

project(line_bezier VERSION 1.0)

add_executable(line_bezier main.c)

target_link_libraries(line_bezier PRIVATE raylib)
