@ECHO OFF
SETLOCAL

set COMMAND=%1
set CONFIG=%2
set BUILD_DIR=_build
set ARCH=x64

if "%CONFIG%"=="" set CONFIG=Debug
if "%COMMAND%" == "" set COMMAND=code

echo Executing build script: Command:%COMMAND% Config:%CONFIG% Architecture:%ARCH%

call :%COMMAND%
echo Build script complete. Error Level: %ERRORLEVEL%
exit /B %ERRORLEVEL%

:clean
    rmdir /s /q %BUILD_DIR%
exit /B %ERRORLEVEL%

:code
    cmake -G"Visual Studio 15 2017" -A%ARCH% -B%BUILD_DIR% .
exit /B %ERRORLEVEL%

:compile
    call :code
    cmake --build "%BUILD_DIR%" --target ALL_BUILD --config "%CONFIG%"
exit /B %ERRORLEVEL%

:test
    call :compile
    cd "%BUILD_DIR%"/tests/
    ctest -C "%CONFIG%" -V
    cd ../../
exit /B %ERRORLEVEL%