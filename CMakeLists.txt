cmake_minimum_required(VERSION 3.15)
project(tight_uint)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fconcepts-diagnostics-depth=32)
endif()

set(HEADERS
    include/tight_uint/tight_uint.hpp
)

add_library(tight_uint INTERFACE ${HEADERS})
target_include_directories(tight_uint INTERFACE include)

enable_testing()
add_subdirectory(tests)
