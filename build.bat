@echo off

REM call buildShaders.bat
call buildBase.bat
call buildEntry.bat

set Warnings=-Wall -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4200 -wd4324 -wd4996 -wd4820 -wd5246 -wd5029 -wd4815 -wd5220 -wd4514 ^
             -wd4577 -wd5045 -wd4471 -wd5262 -wd4711 -wd4710
set IncludeDirs=-I "..\third_party\include"

set CompOpts=%Warnings% %IncludeDirs% -MD -FC -Oi -Od -GR- -EHa- -nologo -std:c++latest

set LnkOpts=-opt:ref -subsystem:windows,5.02

set Win_Libs=User32.lib Gdi32.lib Winmm.lib Shell32.lib Shlwapi.lib hid.lib Ole32.lib Advapi32.lib
set Libs=%Win_Libs% base.lib -LIBPATH:"..\third_party\libs\vulkan" vulkan-1.lib

REM set FileNameTime=%time:~0,8%
REM set FileNameTime=%FileNameTime: =0%
REM set FileNameTime=%date:-=%_%FileNameTime::=%

IF NOT EXIST bin mkdir bin
pushd bin

REM del PDB_Main_DLL_*
REM -PDB:PDB_Main_DLL_%FileNameTime%.pdb

cl %CompOpts% -LD  -I "..\menc_lib\src" ..\src\rhythm_car.cpp -link -OUT:RC_Main_DLL.dll -IMPLIB:RC_Main_DLL.lib ^
   %LnkOpts% %Libs%

cl %CompOpts% -LD  -I "..\menc_lib\src" ..\src\rhythm_car.cpp -link -OUT:RC_Main_DLL.dll -IMPLIB:RC_Main_DLL.lib ^
   %LnkOpts% %Libs%

popd
