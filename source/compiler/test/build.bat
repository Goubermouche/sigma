@echo off
if exist a.exe del a.exe

link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib
a.exe