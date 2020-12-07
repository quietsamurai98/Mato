cmake_minimum_required(VERSION 3.17)
project(cext C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Wall -Wextra -Wno-unknown-pragmas -pedantic -O0 -g -save-temps -DNOT_DRAGONRUBY)

add_executable(cext src/render.c src/terrain.c src/xorshift.c mato.c)
