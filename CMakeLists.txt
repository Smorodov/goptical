project(goptical CXX C)

cmake_minimum_required(VERSION 3.12)
set (CMAKE_CXX_STANDARD 14)

include_directories(include)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(test)