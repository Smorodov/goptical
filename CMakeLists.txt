cmake_minimum_required(VERSION 3.10)
project(goptical CXX C)
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories(PUBLIC include)

add_subdirectory(src)
add_subdirectory(cmd)
add_subdirectory(examples)
#add_subdirectory(test)