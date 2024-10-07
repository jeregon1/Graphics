cmake_minimum_required(VERSION 3.10)
project(Graphics)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 20)

# Add source files
add_executable(Graphics
    src/geometry.cpp
    src/Image.cpp
    src/object3D.cpp
    src/toneMapping.cpp
)

# Add test files
add_executable(runTests
    test/test_geometry.cpp
    test/test_p2.cpp
    test/test_p3.cpp
)

# Include directories
include_directories(include)

# Add GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.10.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Link GoogleTest to the test executable
target_link_libraries(runTests gtest_main)