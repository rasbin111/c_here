cmake_minimum_required(VERSION 3.10)
project(bouncyball)

add_executable(bouncyball main.c)

target_link_libraries(bouncyball raylib)