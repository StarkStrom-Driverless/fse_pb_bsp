@echo off
rem Windows counterpart of fse_pb_bsp/tools/ss.
rem
rem Windows has no usable symlinks without admin rights, so this file gets
rem copied to the project root instead of linked - the same place the ./ss
rem symlink lives on linux. It resolves everything relative to itself, exactly
rem like the bash version resolves against BASH_SOURCE.

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

set "TOOLS=%SCRIPT_DIR%\fse_pb_bsp\tools"

if not exist "%TOOLS%\ss.py" (
    echo error: %TOOLS%\ss.py not found.
    echo        copy ss.cmd to the project root ^(next to the fse_pb_bsp folder^)
    echo        and run it from there.
    exit /b 1
)

set "VENV_PY=%SCRIPT_DIR%\.venv\Scripts\python.exe"

pushd "%TOOLS%"

if exist "%VENV_PY%" (
    "%VENV_PY%" "%TOOLS%\ss.py" %*
) else (
    where /q py
    if !errorlevel! equ 0 (
        py -3 "%TOOLS%\ss.py" %*
    ) else (
        python "%TOOLS%\ss.py" %*
    )
)

set "RC=!errorlevel!"
popd

exit /b %RC%
