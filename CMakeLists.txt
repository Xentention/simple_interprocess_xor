cmake_minimum_required(VERSION 3.25)
project(simple_interprocess_xor C)

set(CMAKE_C_STANDARD 11)

add_executable(program1 program1/src/main.c)
add_executable(program2 program1/src/main.c)
