@echo off

set Warnings=-Wall -WX -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4200 -wd4324 -wd4996 -wd4820 -wd5246 -wd5029 -wd4815 -wd5220 -wd4514 ^
             -wd4577 -wd5045 -wd4471

REM -Z7
set CompOpts=%Warnings% -MD -FC -Oi -O2 -GR- -EHa- -nologo
set LnkOpts=-opt:ref -subsystem:windows,5.02
set Libs=base.lib

IF NOT EXIST bin mkdir bin
pushd bin

cl %CompOpts% ..\src\entry.c -link -OUT:rhythmcar.exe %LnkOpts% %Libs%

popd
