@echo off
title PE parser - build and run
cd /d "%~dp0"
set GCC=D:\w64devkit\bin\gcc.exe

echo ============================================================
echo    Build and run  pe_parser.c
echo ============================================================
echo.

if not exist "%GCC%" (
  echo gcc not found: %GCC%
  pause
  exit /b 1
)

echo --- compiling ---
"%GCC%" -O0 -m64 -o pe_parser.exe pe_parser.c
if errorlevel 1 (
  echo.
  echo COMPILE FAILED - read the error above
  pause
  exit /b 1
)
echo OK
echo.

echo ############################################################
echo  #  1) hello_key.exe  (your practice target)
echo ############################################################
echo.
pe_parser.exe "C:\Users\lzc\Documents\Codex\2026-09-17\w\outputs\crackme_practice\hello_key.exe"
echo.
echo ############################################################
echo  #  2) notepad.exe  (a real Windows program)
echo ############################################################
echo.
pe_parser.exe "C:\Windows\System32\notepad.exe"
echo.
echo ############################################################
echo  #  Compare the two  e_lfanew  values above!
echo  ############################################################
echo.
pause