@echo off
call "%~dp0AVVIA_RICERCA.bat" --no-download --insert %*
exit /b %errorlevel%
