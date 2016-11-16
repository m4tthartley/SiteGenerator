@echo off

pushd build
cl -Zi -Od -MTd -nologo ../code/main.cc -Fewebsitegenerator.exe -link -subsystem:console
popd

pushd websites\matt
build.bat
popd