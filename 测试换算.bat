@echo off
title PE parser - RVA to file offset
cd /d "%~dp0"

set TARGET=C:\Users\lzc\Documents\Codex\2026-09-17\w\outputs\crackme_practice\hello_key.exe

if not exist pe_parser.exe (
  echo pe_parser.exe not found. Run ????.bat first.
  pause
  exit /b 1
)

echo ============================================================
echo   Test 1:  RVA 0x16B4   (the jne / sheng-si-men)
echo ============================================================
echo.
pe_parser.exe "%TARGET%" --rva 0x16B4
echo.
echo ============================================================
echo   Test 2:  RVA 0xA011   (the "%%04X" string)
echo ============================================================
echo.
pe_parser.exe "%TARGET%" --rva 0xA011
echo.
echo ============================================================
echo   Test 3:  RVA 0x0      (is it inside any section?)
echo ============================================================
echo.
pe_parser.exe "%TARGET%" --rva 0x0
echo.
echo ============================================================
echo   Test 4:  RVA 0x12345678  (way out of range)
echo ============================================================
echo.
pe_parser.exe "%TARGET%" --rva 0x12345678
echo.
echo ============================================================
echo  Now try your own numbers!  Edit this file and change 0x16B4
echo  to anything, or run it by hand:
echo.
echo    pe_parser.exe "%TARGET%" --rva 0xA020
echo ============================================================
echo.
pause