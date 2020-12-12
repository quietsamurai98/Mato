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
COMPILER_FLAGS="-isysroot $(xcrun --show-sdk-path) $(clang -E -xc++ -Wp,-v /dev/null 2>&1 | sed -n '/^#include <...>/,/^End of search/p'| sed '1d;$d;s/\/\(.*\)/-I \/\1/;s/ (framework directory)//') -isystem ./include -I. -I /usr/local/Cellar/libomp/11.0.0/include/ -L/usr/local/Cellar/libomp/11.0.0/lib/ -Ofast -flto -Xpreprocessor -fopenmp -lomp"
./dragonruby-bind --compiler-flags="$COMPILER_FLAGS" --ffi-module=MatoCore --output=./mygame/native/mato-bind.c ./mygame/cext/mato.c
clang $COMPILER_FLAGS -v -fPIC -shared ./mygame/cext/src/*.c ./mygame/native/mato-bind.c -o ./mygame/native/$PLATFORM/matocore.$DLLEXT
