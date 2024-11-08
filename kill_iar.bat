@echo off

set iar_path=prj
rd %~dp0\%iar_path%\Debug /s/q
rd %~dp0\%iar_path%\Release /s/q
rd %~dp0\%iar_path%\settings /s/q
 


set iar_path=bsp\bootloader\build
rd %~dp0\%iar_path%\Debug /s/q
rd %~dp0\%iar_path%\Release /s/q
rd %~dp0\%iar_path%\settings /s/q
 


 