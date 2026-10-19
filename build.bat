@echo off
chcp 65001 >nul
setlocal

if not exist build mkdir build
if not exist data mkdir data

gcc -std=c11 -Wall -Wextra -Wpedantic ^
    -Iinclude -Isrc ^
    src\inventory.c src\storage.c src\main.c ^
    -o build\warehousems.exe

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build successful: build\warehousems.exe
endlocal