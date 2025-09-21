cmake_minimum_required(VERSION 3.10)
project(MyProject C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(SDL2 REQUIRED)

find_package(SDL2_ttf REQUIRED)

find_package(OpenGL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)

include_directories(${GLIB2_INCLUDE_DIRS})

add_definitions(${GLIB2_CFLAGS_OTHER})

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.h")

add_executable(MyProject ${SOURCES})

target_link_libraries(MyProject PRIVATE 
    SDL2::SDL2 
    SDL2_ttf::SDL2_ttf 
    OpenGL::GL
    ${GLIB2_LIBRARIES}
)

target_include_directories(MyProject PRIVATE ${GLIB2_INCLUDE_DIRS})
target_compile_options(MyProject PRIVATE ${GLIB2_CFLAGS_OTHER})
