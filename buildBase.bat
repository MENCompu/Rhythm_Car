@echo off

set Warnings=-Wall -WX -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4200 -wd4324 -wd4996 -wd4820 -wd5246 -wd5029 -wd4815 -wd5220 -wd4514 ^
             -wd4577 -wd5045 -wd4471
set IncludeDirs=-I "C:\VulkanSDK\1.3.268.0\Include"

REM -Z7
set CompOpts=%Warnings% %IncludeDirs% -MD -FC -Oi -Od -GR- -EHa- -nologo
set LnkOpts=-opt:ref -subsystem:windows,5.02

set Win_Libs=User32.lib Gdi32.lib Winmm.lib Shell32.lib Shlwapi.lib
set Libs=%Win_Libs% -LIBPATH:"C:\VulkanSDK\1.3.268.0\Lib" vulkan-1.lib

IF NOT EXIST bin mkdir bin
pushd bin

cl %CompOpts% -LD ..\src\base_bin.cpp -link -OUT:base.dll -IMPLIB:base.lib %LnkOpts% %Libs%

popd
