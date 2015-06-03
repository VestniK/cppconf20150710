project(cli)
cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_definitions(-Wall -Werror -std=c++14)

set(SRC
  main.cpp
)

add_executable(cli ${SRC})
