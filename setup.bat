REM Buduje projekt za pomocą CMake i uruchamia program

if not exist build mkdir build
cd build
cmake ..
cmake --build . --config Debug
if %errorlevel% neq 0 (
    echo Kompilacja nie powiodla sie!
    exit /b %errorlevel%
)
cd Debug
outDebug.exe
