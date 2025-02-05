cmake_minimum_required(VERSION 3.20)
project(code C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# This is a sudo-library that captures all the build options.
# It makes it easy to re-use the same options for multiple targets, without
# setting them globally which would apply them to any dependencies too.
add_library(compile-options INTERFACE)

# Add common compiler warnings
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(compile-options INTERFACE -Wall -Wextra -Wconversion)# -Werror)
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(compile-options INTERFACE /TC /W4 /wd4200 /WX)  # c mode, disable error from flexible array member.
endif ()

# Enable AddressSanitizer in debug mode when using Clang on linux or macos.
# * On MacOS need environment variable set MallocNanoZone=0
#   or you'll get a warning with asan in stdlib code.
if ((NOT CMAKE_SYSTEM_NAME STREQUAL "Windows") AND (CMAKE_C_COMPILER_ID STREQUAL "Clang") AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    target_compile_options(compile-options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(compile-options INTERFACE -fsanitize=address)
endif ()

enable_testing()

add_executable(test_steve ../test_steve.c)
target_link_libraries(test_steve compile-options)
add_test(NAME test_steve COMMAND test_steve)

if (NOT (CMAKE_C_COMPILER_ID STREQUAL "MSVC"))
    add_executable(dandd ../dandd.c)
    target_link_libraries(dandd compile-options)
    add_test(NAME dandd COMMAND dandd)
endif ()

# Stuff in lab doesn't necessarily work on all platforms or even compile.
if (NOT ((CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_C_COMPILER_ID MATCHES "Clang|MSVC")))
    add_executable(simd ../lab/simd_single_file_gcc_clang.c)
    target_link_libraries(simd compile-options)
    add_test(NAME simd COMMAND simd)
endif ()

if (NOT ((CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_C_COMPILER_ID MATCHES "Clang|MSVC")))
    add_executable(bigints ../lab/bigints.c)
    target_link_libraries(bigints compile-options)
    add_test(NAME bigints COMMAND bigints)
endif ()

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE) # don't build the supplied example games
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD OFF CACHE BOOL "" FORCE)


FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(tazar ../tazar/tazar_game.c ../tazar/tazar_ui.c ../tazar/tazar_main.c)
target_include_directories(tazar PRIVATE ../3rdparty/bestline)
target_include_directories(tazar PRIVATE ..)
target_link_libraries(tazar PRIVATE compile-options raylib)

#add_library(bestline ../3rdparty/bestline/bestline.c)
#target_link_libraries(tazar compile-options bestline)