cmake_minimum_required(VERSION 3.0)
project(wordle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined")

include(cmake/uWebSockets.cmake)

add_executable(CalcBruteForce src/calc_brute_force.cpp)

add_executable(ConsoleApp src/console_app.cpp)

add_executable(WebApp src/web_app.cpp)
target_link_libraries(WebApp PRIVATE uWebSockets)
