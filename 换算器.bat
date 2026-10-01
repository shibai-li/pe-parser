@echo off
title RVA converter
cd /d "%~dp0"

set TARGET=C:\Users\lzc\Documents\Codex\2026-09-17\w\outputs\crackme_practice\hello_key.exe

if not exist pe_parser.exe (
  echo pe_parser.exe not found. Run ????.bat first.
  pause
  exit /b 1
)

:again
cls
echo ============================================================
echo    RVA  -^>  file offset      (hello_key.exe)
echo ============================================================
echo.
echo  Some RVAs you can try:
echo    0x16B4   the jne  (sheng-si-men)
echo    0xA011   the "%%04X" string
echo    0xA000   start of .rdata
echo    0x1000   start of .text
echo    0x0      not inside any section
echo.
set /p "RVA= Type an RVA (or just press Enter to quit): "
if "%RVA%"=="" goto end

echo.
pe_parser.exe "%TARGET%" --rva %RVA%
echo.
echo ------------------------------------------------------------
pause
goto again

:end