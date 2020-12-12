#!/bin/bash

OSTYPE=`uname -s`
if [ "x$OSTYPE" = "xDarwin" ]; then
  PLATFORM=macos
  DLLEXT=dylib
else
  PLATFORM=linux-amd64
  DLLEXT=so
fi

cd ../../
mkdir -p ./mygame/native/$PLATFORM
COMPILER_FLAGS="-isystem ./include -I. -Ofast -flto=full -fopenmp"
./dragonruby-bind --compiler-flags="$COMPILER_FLAGS" --ffi-module=MatoCore --output=./mygame/native/mato-bind.c ./mygame/cext/mato.c
clang $COMPILER_FLAGS -fPIC -shared ./mygame/cext/src/*.c ./mygame/native/mato-bind.c -o ./mygame/native/$PLATFORM/matocore.$DLLEXT
