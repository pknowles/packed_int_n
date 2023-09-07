cmake_minimum_required(VERSION 3.15)
project(packed_int_n)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fconcepts-diagnostics-depth=32)
endif()

set(HEADERS
    include/packed_intn/packed_intn.hpp
)

add_library(packed_intn INTERFACE ${HEADERS})
target_include_directories(packed_intn INTERFACE include)

enable_testing()
add_subdirectory(tests)
