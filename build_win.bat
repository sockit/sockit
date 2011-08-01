@echo off

cd ..\.. 

:: replace this with prep20xx that you'd like to use
CALL prep2010.cmd

cd build

CALL msbuild ALL_BUILD.vcxproj
