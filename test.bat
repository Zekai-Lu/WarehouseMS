@echo off
setlocal

if not exist build mkdir build

gcc -std=c11 -Wall -Wextra -Wpedantic ^
    -Iinclude -Isrc ^
    src\inventory.c src\storage.c tests\test_inventory.c ^
    -o build\test_inventory.exe

if errorlevel 1 (
    echo Test compilation failed.
    exit /b 1
)

build\test_inventory.exe
if errorlevel 1 (
    echo Tests failed.
    exit /b 1
)

endlocal