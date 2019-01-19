cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(ree_image CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ree_image main.cc)

add_executable(ree_image_test test.cc)