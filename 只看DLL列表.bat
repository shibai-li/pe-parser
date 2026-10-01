@echo off
title Imports - DLL list only
cd /d "%~dp0"

set HK=C:\Users\lzc\Documents\Codex\2026-09-17\w\outputs\crackme_practice\hello_key.exe
set NP=C:\Windows\System32\notepad.exe

echo ============================================================
echo   Only the DLL names (function names are filtered out)
echo ============================================================
echo.
echo ---------- hello_key.exe ----------
pe_parser.exe "%HK%" | findstr /B /C:"--- "
echo.
echo ---------- notepad.exe ----------
pe_parser.exe "%NP%" | findstr /B /C:"--- "
echo.
echo ============================================================
echo  That's it. Each "--- xxx.dll ---" line = one DLL.
echo ============================================================
echo.
pause