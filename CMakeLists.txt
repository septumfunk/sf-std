cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SF_LIBRARY_TYPE STATIC)
if (BUILD_SHARED_LIBS)
    set(SF_LIBRARY_TYPE SHARED)
endif()

project(sf-structures C)

add_library(sf-structures ${SF_LIBRARY_TYPE}
    src/str.c
    src/dynamic.c
)
target_include_directories(sf-structures PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(sf-structures PRIVATE
    -Wall -Werror -Wextra -pedantic -Wconversion
    -Wsign-conversion -Wformat=2 -Wundef
    -Wold-style-cast -Woverloaded-virtual
    -Wdouble-promotion -Wnon-virtual-dtor
    -Wnull-dereference -Wstrict-overflow
)

if (WIN32)
    if (BUILD_SHARED_LIBS)
        target_compile_definitions(sf-structures PUBLIC SF_DYNAMIC)
        target_compile_definitions(sf-structures PRIVATE SF_EXPORTS)
    endif()
endif()
