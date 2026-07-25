@echo off
REM fm.bat — global launcher for the FM file manager (Windows).
REM
REM %~dp0 always expands to the folder this .bat file lives in, so this
REM works from any current directory once that folder (or a copy of
REM this file) is on PATH.
REM
REM Install: see README.md, section "Run FM from anywhere".

setlocal
set "FM_DIR=%~dp0"
cd /d "%FM_DIR%"

if exist "%FM_DIR%.venv\Scripts\python.exe" (
    "%FM_DIR%.venv\Scripts\python.exe" run.py %*
) else (
    python run.py %*
)
endlocal
