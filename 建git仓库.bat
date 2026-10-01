@echo off
title git - create repo
cd /d "%~dp0"

echo ============================================================
echo   Step 1 of 5 : git identity (local to this repo)
echo ============================================================
echo.
set "GN="
set /p "GN= Your name for git commits (e.g. lzc), Enter = use lzc : "
if "%GN%"=="" set "GN=lzc"

set "GM="
set /p "GM= Your email, Enter = use lzc@localhost : "
if "%GM%"=="" set "GM=lzc@localhost"

git config user.name  "%GN%"
git config user.email "%GM%"
git config core.autocrlf false
git config core.quotepath false
echo.
echo   user.name  = %GN%
echo   user.email = %GM%
echo.
echo   (Change later:  git config user.name "xxx"   -- no --global needed)
echo.

echo ============================================================
echo   Step 2 of 5 : git init
echo ============================================================
if exist .git (
  echo   already a git repo
) else (
  git init
)
echo.

echo ============================================================
echo   Step 3 of 5 : .gitignore
echo ============================================================
if exist .gitignore ( echo   .gitignore OK ) else ( echo   WARNING: no .gitignore )
echo.

echo ============================================================
echo   Step 4 of 5 : add + commit
echo ============================================================
git add .
git commit -m "first commit: Windows PE parser (section table + import table + RVA conversion)"
echo.

echo ============================================================
echo   Step 5 of 5 : result
echo ============================================================
echo.
echo --- files in the repo ---
git ls-files
echo.
echo --- commit history ---
git log --oneline
echo.
echo --- status ---
git status --short
echo.
echo ============================================================
echo   Done.
echo   Next: create a repo on GitHub, then we push.
echo ============================================================
pause