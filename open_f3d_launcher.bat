@echo off
setlocal enabledelayedexpansion

:: Fusion 360 F3D File Launcher
:: Uses D drive real path to avoid mklink symlink IPC issues
:: Dynamically finds the latest version of FusionLauncher.exe

set "BASE_DIR=D:\Fusion360\Installation\Local\Autodesk\webdeploy\production"

:: Find the latest version directory containing FusionLauncher.exe
set "LATEST_LAUNCHER="
for /f "delims=" %%D in ('dir /b /ad /o-n "%BASE_DIR%" 2^>nul') do (
    if exist "%BASE_DIR%\%%D\FusionLauncher.exe" (
        if not defined LATEST_LAUNCHER (
            set "LATEST_LAUNCHER=%BASE_DIR%\%%D\FusionLauncher.exe"
        )
    )
)

:: If not found on D drive, try symlink path (C drive)
if not defined LATEST_LAUNCHER (
    set "BASE_DIR=C:\Users\G11\AppData\Local\Autodesk\webdeploy\production"
    for /f "delims=" %%D in ('dir /b /ad /o-n "%BASE_DIR%" 2^>nul') do (
        if exist "%BASE_DIR%\%%D\FusionLauncher.exe" (
            if not defined LATEST_LAUNCHER (
                set "LATEST_LAUNCHER=%BASE_DIR%\%%D\FusionLauncher.exe"
            )
        )
    )
)

:: If still not found, exit with error
if not defined LATEST_LAUNCHER (
    echo Error: Cannot find FusionLauncher.exe
    exit /b 1
)

:: Launch FusionLauncher.exe with the file path argument
start "" "%LATEST_LAUNCHER%" "%~1"

endlocal
