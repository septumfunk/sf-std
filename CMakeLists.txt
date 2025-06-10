cmake_minimum_required(VERSION 3.28)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SF_LIBRARY_TYPE STATIC)
if (BUILD_SHARED_LIBS)
    set(SF_LIBRARY_TYPE SHARED)
endif()

project(sf-std C)

add_library(sf-std ${SF_LIBRARY_TYPE}
    src/str.c
    src/dynamic.c
    src/numerics.c
    src/fs.c
)
target_include_directories(sf-std PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(sf-std PRIVATE
    -Wall -Werror -Wextra -pedantic -Wconversion
    -Wsign-conversion -Wformat=2 -Wundef
    -Wold-style-cast -Woverloaded-virtual
    -Wdouble-promotion -Wnon-virtual-dtor
    -Wnull-dereference -Wstrict-overflow
)

if (WIN32)
    if (BUILD_SHARED_LIBS)
        set(CMAKE_SHARED_LIBRARY_PREFIX "")
        target_compile_definitions(sf-std PUBLIC SF_DYNAMIC)
        target_compile_definitions(sf-std PRIVATE SF_EXPORTS)
    endif()
endif()
