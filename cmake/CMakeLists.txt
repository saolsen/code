cmake_minimum_required(VERSION 3.30)
project(code C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(test_steve ../test_steve.c)

# Add common compiler warnings
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_steve PRIVATE -Wall -Wextra -Werror -Wconversion)
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(test_steve PRIVATE /TC /W4 /wd4200 /WX)  # c mode, disable error from flexible array member.
endif()

# Enable AddressSanitizer in debug mode when using Clang on linux or macos.
# * On MacOS need environment variable set MallocNanoZone=0
#   or you'll get a warning with asan in stdlib code.
if (
  (NOT CMAKE_SYSTEM_NAME MATCHES "Windows") AND
  (CMAKE_C_COMPILER_ID MATCHES "Clang") AND
  (CMAKE_BUILD_TYPE STREQUAL "Debug")
)
    target_compile_options(test_steve PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(test_steve PRIVATE -fsanitize=address)
endif()
