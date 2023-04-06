cmake_minimum_required(VERSION 3.11.0)
project(pmi-process-progress VERSION 0.1.0)

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
find_package(range-v3 REQUIRED)
find_package(Clang REQUIRED)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
message(${CLANG_INCLUDE_DIRS})
execute_process(COMMAND uname -r OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "-Wall -g -pthread -lrt -rdynamic")
set(CMAKE_CXX_STANDARD 23)

add_executable(pmi-process-progress ${SOURCE_FILES})

include_directories(pmi-process-progress include)
target_link_libraries(pmi-process-progress cxxopts::cxxopts fmt::fmt range-v3::range-v3 elf bpf)
