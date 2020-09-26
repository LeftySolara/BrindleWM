cmake_minimum_required(VERSION 3.6)

project(
    brindlewm
    DESCRIPTION "A basic X11 window manager."
    HOMEPAGE_URL "https://git.jalenkadams.me/LeftySolara/brindlewm"
    LANGUAGES C
    VERSION 0.1
)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Werror -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(src)

# Recursively add source files to the build tree.
file(GLOB_RECURSE SOURCE_FILES "src/*.c")
add_executable(brindlewm ${SOURCE_FILES})

set_target_properties(brindlewm PROPERTIES OUTPUT_NAME "brindlewm")
target_link_libraries(brindlewm -lxcb -lxcb-util)
