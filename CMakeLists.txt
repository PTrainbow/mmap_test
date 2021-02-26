cmake_minimum_required(VERSION 3.14)
project(Learn_mmap)

set(CMAKE_CXX_STANDARD 14)

set(SRC_LIST main.c)

add_executable(Learn_mmap  ${SRC_LIST})
