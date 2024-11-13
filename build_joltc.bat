@echo off

IF NOT DEFINED clset (call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat")

set exe_name="Cobble"
set build_options=/D "BUILD_WIN32=1"

set debug_flags=/Od /Oi /GR /Zo /Z7

set compile_flags=-nologo -std:c++17 /EHsc /MTd %debug_flags%

set link_win32=User32.lib winmm.lib kernel32.lib gdi32.lib shell32.lib 
set link_jolt=Jolt.lib joltc.lib
set link_flags=/link %link_win32% %link_jolt%

set include_dir=/I ../include/
set core=../include/JoltC/JoltC.cpp

if not exist build mkdir build
pushd build
cl %build_options% %compile_flags% %core% %include_dir% %link_flags% %lib_paths% /out:%exe_name%.exe
copy ..\data\* . >NUL
popd

pause