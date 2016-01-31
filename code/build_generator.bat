@echo off
pushd ..\build

cl -Zi -Od -MTd ../code/main.c -Fesitegen.exe -link -subsystem:console

popd