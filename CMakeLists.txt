enable_language(C)
enable_language(CXX)
cmake_minimum_required(VERSION 3.0)
project(LigmaLang CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Add the LLVM include directories
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
# Add source files
add_executable(MyExecutable main.cpp)
target_include_directories(MyExecutable PRIVATE include)
# Link against LLVM
target_link_libraries(MyExecutable PRIVATE LLVM)
