@echo off
setlocal EnableExtensions
cd /d "%~dp0"
set "MSYSTEM=MINGW64"
set "CHERE_INVOKING=1"
set "MSYS_ROOT="
if defined MSYS2_ROOT if exist "%MSYS2_ROOT%\usr\bin\bash.exe" set "MSYS_ROOT=%MSYS2_ROOT%"
if not defined MSYS_ROOT if exist "C:\msys64\usr\bin\bash.exe" set "MSYS_ROOT=C:\msys64"
if not defined MSYS_ROOT if exist "%ProgramFiles%\msys64\usr\bin\bash.exe" set "MSYS_ROOT=%ProgramFiles%\msys64"
if not defined MSYS_ROOT if exist "%LOCALAPPDATA%\Programs\MSYS2\usr\bin\bash.exe" set "MSYS_ROOT=%LOCALAPPDATA%\Programs\MSYS2"
if not defined MSYS_ROOT (
  echo [DIAGNOSTICA] MSYS2 gia installato non trovato: imposta MSYS2_ROOT.
  exit /b 2
)
"%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash sorgenti/tests/run_diagnostic_runtime.sh"
exit /b %ERRORLEVEL%
