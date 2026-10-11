cmake_minimum_required(VERSION 3.25)

project(uchat
    VERSION 1.0.0
    DESCRIPTION "Qt Desktop Application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

file(GLOB_RECURSE SOURCES 
    CONFIGURE_DEPENDS 
    "src/*.cpp" 
    "src/*.hpp"
)

qt_add_executable(uchat ${SOURCES})

target_include_directories(uchat PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_options(uchat PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wformat=2
    >
    $<$<CXX_COMPILER_ID:GNU>:
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wformat=2
    >
)

target_link_libraries(uchat PRIVATE Qt6::Widgets)