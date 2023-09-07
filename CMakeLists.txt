cmake_minimum_required(VERSION 3.15)
project(packed_int_n)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
    include/packed_intn/packed_intn.hpp
)

add_library(packed_intn INTERFACE ${HEADERS})
target_include_directories(packed_intn INTERFACE include)

enable_testing()
add_subdirectory(tests)
