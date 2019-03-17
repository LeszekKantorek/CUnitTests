@ECHO OFF
SETLOCAL

set COMMAND=%1
set CONFIG=%2
set BUILD_DIR=_build
set ARCH=x64

if "%CONFIG%"=="" set CONFIG=Debug
IF "%COMMAND%" == "" set COMMAND=code

echo PARAMS Command:"%COMMAND%" Config:"%CONFIG%" Arch:"%ARCH%" Directory:"%BUILD_DIR%"

call :%COMMAND%
echo Build script complete. Error Level: %ERRORLEVEL%
exit /B %ERRORLEVEL%

:clean
    rmdir /s /q %BUILD_DIR%
exit /B %ERRORLEVEL%

:code
    cmake -G"Visual Studio 15 2017" -A%ARCH% -B%BUILD_DIR% -H.
exit /B %ERRORLEVEL%

:compile
    call :code
    cmake --build "%BUILD_DIR%" --target ALL_BUILD --config "%CONFIG%"
exit /B %ERRORLEVEL%

:test
    call :compile
    cd "%BUILD_DIR%" 
    ctest -C "%CONFIG%" -V
exit /B %ERRORLEVEL%

