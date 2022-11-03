cmake_minimum_required(VERSION 3.18.0)

set(CMAKE_C_STANDARD 11)

project(cpu-usage-tracker)

if (${CMAKE_C_COMPILER_ID} STREQUAL Clang)
    add_compile_options(-Weverything)
elseif(${CMAKE_C_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wstrict-prototypes -Wmissing-prototypes")
endif()

include(FetchContent)
FetchContent_Declare(gtest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG release-1.12.1)
FetchContent_GetProperties(gtest)

if (NOT gtest_POPULATED)
    FetchContent_Populate(gtest)
    add_subdirectory(${gtest_SOURCE_DIR} ${gtest_BUILD_DIR})
endif()

find_package(Threads REQUIRED)

add_executable(CUT
    src/main.c
    src/Reader.c
    src/Analyzer.c
    src/Printer.c
    src/Logger.c
    src/SafeQueue.c)
target_include_directories(CUT PUBLIC include)
target_link_libraries(CUT pthread)

# cmake .. -DBUILD_TESTS:BOOL=OFF
SET(BUILD_TESTS "ON" CACHE BOOL "ON/OFF option for building tests")

if (BUILD_TESTS)
    add_executable(tests
        tests/tests.cpp
        src/Reader.c
        tests/Reader_TEST.cpp)
    target_include_directories(tests PUBLIC include src)
    target_link_libraries(tests pthread gtest_main gmock_main)
endif()

