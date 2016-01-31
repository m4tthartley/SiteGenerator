@echo off
pushd ..\build

cl -Zi -Od -MTd ../code/server.c -Feserver.exe -link -subsystem:console Ws2_32.lib Shlwapi.lib

popd