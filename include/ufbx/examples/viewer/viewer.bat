@echo off

IF NOT DEFINED clset (call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat")

set exe_name="UFBX_Viewer"
set build_options=/D "BUILD_WIN32=1"

set debug_flags=/Od /Oi /GR /Zo /Z7
set compile_flags=-nologo -std:c++17 -Zc:__cplusplus -Zc:preprocessor /EHsc /MTd %debug_flags%

set lib_paths=/libpath:..\lib /libpath:..\third_party\joltc\lib\Release\ /libpath:..\lib\glew\ /libpath:..\lib\glfw\ /libpath:..\lib\bgfx\
set link_win32=User32.lib winmm.lib kernel32.lib gdi32.lib shell32.lib 
set link_gl=opengl32.lib
set link_flags=/link %link_win32% %link_gl% /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:LIBCMT

set include_dir=/I ../shaders/ /I ../external/ 
set ufbx=../../../ufbx.c
set external=../external.c
set core=../viewer.c

if not exist build mkdir build
pushd build
cl %build_options% %compile_flags% %core% %external% %ufbx% %include_dir% %link_flags% %lib_paths% /out:%exe_name%.exe
copy ..\data\* . >NUL
popd

pause