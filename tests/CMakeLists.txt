# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.15)

# Include Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(${PROJECT_NAME}_tests
    test_main.cpp
)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE
    packed_intn
    gtest_main
)

target_compile_options(${PROJECT_NAME}_tests PRIVATE -Wall -Wextra -pedantic)

# Enable testing with CTest
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)
