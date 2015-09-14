@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

set PATH=W:\misc;"C:\Program Files (x86)\Vim\vim74";%path%

start gvim

w:
