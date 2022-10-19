cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)

project(cpu-usage-tracker)

add_executable(CUT
    src/main.c
    src/Reader.c)

target_include_directories(CUT PUBLIC include)