cmake_minimum_required(VERSION 3.0)
project(wordle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined")

add_executable(Wordle main.cpp)
