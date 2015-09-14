@echo off

pushd w:\clib\build
cl -Zi w:\clib\main.cpp
cl -Zi w:\clib\generator.cpp
popd
