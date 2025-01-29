cmake_minimum_required(VERSION 3.30)
project(code C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# This is a sudo-library that captures all the build options.
# It makes it easy to re-use the same options for multiple targets, without
# setting them globally which would apply them to any dependencies too.
add_library(compile-options INTERFACE)

# Add common compiler warnings
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(compile-options INTERFACE -Wall -Wextra -Werror -Wconversion)
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(compile-options INTERFACE /TC /W4 /wd4200 /WX)  # c mode, disable error from flexible array member.
endif ()

# Enable AddressSanitizer in debug mode when using Clang on linux or macos.
# * On MacOS need environment variable set MallocNanoZone=0
#   or you'll get a warning with asan in stdlib code.
if (
(NOT CMAKE_SYSTEM_NAME MATCHES "Windows") AND
(CMAKE_C_COMPILER_ID MATCHES "Clang") AND
(CMAKE_BUILD_TYPE STREQUAL "Debug")
)
    target_compile_options(compile-options INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(compile-options INTERFACE -fsanitize=address)
endif ()

enable_testing()

add_executable(test_steve ../test_steve.c)
target_link_libraries(test_steve compile-options)
add_test(NAME test_steve COMMAND test_steve)

add_executable(dandd ../dandd.c)
target_link_libraries(dandd compile-options)
add_test(NAME dandd COMMAND dandd)

# Stuff in lab doesn't necessarily work on all platforms or even compile.

if (NOT (
(CMAKE_C_COMPILER_ID MATCHES "MSVC") OR
(CMAKE_SYSTEM_NAME MATCHES "Windows" AND CMAKE_C_COMPILER_ID MATCHES "Clang")
))
    add_executable(simd ../lab/simd_single_file_gcc_clang.c)
    target_link_libraries(simd compile-options)
    add_test(NAME simd COMMAND simd)
endif ()

add_executable(bigints ../lab/bigints.c)
target_link_libraries(bigints compile-options)
#add_test(NAME bigints COMMAND bigints)