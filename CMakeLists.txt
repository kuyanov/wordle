cmake_minimum_required(VERSION 3.0)
project(wordle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined")

find_library(USOCKETS_LIB uSockets.a)

add_executable(ConsoleApp src/console_app.cpp)
add_executable(WebApp src/web_app.cpp)

target_compile_definitions(WebApp PRIVATE UWS_NO_ZLIB)
target_link_libraries(WebApp PRIVATE ${USOCKETS_LIB})
