@echo off
setlocal

echo Creating compile_commands.json symlink for language server...
echo.
if exist "%~dp0compile_commands.json" (
    del /f /q "%~dp0compile_commands.json"
)
mklink "%~dp0compile_commands.json" "%~dp0build\windows\compile_commands.json"
echo.

if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\vcpkg.exe" (
        echo VCPKG_ROOT exists: %VCPKG_ROOT%
        echo.
        echo Done!
        exit /b 0
    ) else (
        echo VCPKG_ROOT is defined, but vcpkg.exe was not found inside it.
    )
) else (
    echo VCPKG_ROOT is not defined.
    echo.
)

set "LOCAL_VCPKG_DIR=%~dp0.vcpkg"

if exist "%LOCAL_VCPKG_DIR%\vcpkg.exe" (
    echo Local Vcpkg installation exists: %LOCAL_VCPKG_DIR%
    echo.
    echo Done!
    exit /b 0
)

:: ">nul >2&1" silences output
git --version >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Git is not installed.
    exit /b 1
)

if not exist "%LOCAL_VCPKG_DIR%" (
    echo Cloning Vcpkg locally...
    echo.
    git clone --depth 1 https://github.com/microsoft/vcpkg.git "%LOCAL_VCPKG_DIR%"
    echo.
    if %ERRORLEVEL% neq 0 (
        echo Failed to clone Vcpkg.
        exit /b 1
    )
)

echo Bootstapping local Vcpkg...
echo.
call "%LOCAL_VCPKG_DIR%\bootstrap-vcpkg.bat" -disableMetrics
echo.
if %ERRORLEVEL% neq 0 (
    echo Failed to bootstrap local Vcpkg.
    exit /b 1
)

echo Done!
exit /b 0