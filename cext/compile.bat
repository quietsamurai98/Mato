set DRB_ROOT=..\..
md %DRB_ROOT%\mygame\native
md %DRB_ROOT%\mygame\native\windows-amd64
set COMP_FLAGS_1=--sysroot=C:\mingw-w64\mingw64 --target=x86_64-w64-mingw32 -fuse-ld=lld
set COMP_FLAGS_2=-isystem %DRB_ROOT%\include -I. -I %DRB_ROOT%\mygame\cext
set COMP_FLAGS_3=-fopenmp=libgomp -ferror-limit=9994
set COMP_FLAGS_4=-O0 -g
set COMP_FLAGS_5=-Ofast -flto
set COMP_FLAGS=%COMP_FLAGS_1% %COMP_FLAGS_2% %COMP_FLAGS_3% %COMP_FLAGS_5%

set DRBB_FLAGS=--compiler-flags="%COMP_FLAGS%" --ffi-module=MatoCore --output=%DRB_ROOT%\mygame\native\ext-bind.c %DRB_ROOT%\mygame\cext\mato.c
set C_FILES=%DRB_ROOT%\mygame\cext\src\*.c
%DRB_ROOT%\dragonruby-bind.exe %DRBB_FLAGS%
clang -shared %C_FILES% %DRB_ROOT%\mygame\native\ext-bind.c -o %DRB_ROOT%\mygame\native\windows-amd64\ext.dll %COMP_FLAGS%
