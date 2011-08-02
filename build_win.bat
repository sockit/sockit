@echo off

cd ..\.. 

:: replace this with prep20xx that you'd like to use
CALL prep2010.cmd

cd build

CALL msbuild ALL_BUILD.vcxproj

:: NOTE: You will need to update this 'sockit' path to be the path of the directory into
::   which you cloned the sockit repo, if it is not the default
cd ..\projects\sockit 

@echo off

copy /Y ..\..\build\bin\SockIt\Debug\npSockIt.dll bin\win

@ECHO on

