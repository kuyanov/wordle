cmake_minimum_required(VERSION 3.0)
project(wordle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined")

find_package(Threads REQUIRED)
find_library(USOCKETS_LIBRARY uSockets.a REQUIRED)

add_executable(Benchmark src/benchmark.cpp)
target_link_libraries(Benchmark PRIVATE ${CMAKE_THREAD_LIBS_INIT})

add_executable(ConsoleApp src/console_app.cpp)

add_executable(WebApp src/web_app.cpp)
target_compile_definitions(WebApp PRIVATE UWS_NO_ZLIB)
target_link_libraries(WebApp PRIVATE ${USOCKETS_LIBRARY})
