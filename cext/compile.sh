#!/bin/bash
cd ../../
mkdir -p ./mygame/native
mkdir -p ./mygame/native/linux-amd64
COMPILER_FLAGS="-isystem ./include -I. -Ofast -flto -fopenmp"
./dragonruby-bind --compiler-flags="$COMPILER_FLAGS" --ffi-module=MatoCore --output=./mygame/native/ext-bind.c ./mygame/cext/mato.c
clang $COMPILER_FLAGS -fPIC -shared ./mygame/cext/src/*.c ./mygame/native/ext-bind.c -o ./mygame/native/linux-amd64/ext.so
