@echo off
REM build.bat - For Windows builds

if not exist "%PREFIX%\bin" mkdir "%PREFIX%\bin"
if not exist "%PREFIX%\lib" mkdir "%PREFIX%\lib"
if not exist "%PREFIX%\v8" mkdir "%PREFIX%\v8"

copy bin\plugify-module-v8.dll "%PREFIX%\bin\" || exit 1
xcopy lib "%PREFIX%\lib" /E /Y /I || exit 1
xcopy v8 "%PREFIX%\v8" /E /Y /I || exit 1
copy plugify-module-v8.pmodule "%PREFIX%\" || exit 1

REM Create activation scripts
if not exist "%PREFIX%\etc\conda\activate.d" mkdir "%PREFIX%\etc\conda\activate.d"
if not exist "%PREFIX%\etc\conda\deactivate.d" mkdir "%PREFIX%\etc\conda\deactivate.d"

echo @echo off > "%PREFIX%\etc\conda\activate.d\plugify-module-v8.bat"
echo set "PLUGIFY_V8_MODULE_PATH=%%CONDA_PREFIX%%;%%PLUGIFY_V8_MODULE_PATH%%" >> "%PREFIX%\etc\conda\activate.d\plugify-module-v8.bat"

echo @echo off > "%PREFIX%\etc\conda\deactivate.d\plugify-module-v8.bat"
echo set "PLUGIFY_V8_MODULE_PATH=%%PLUGIFY_V8_MODULE_PATH:%%CONDA_PREFIX%%;=%%" >> "%PREFIX%\etc\conda\deactivate.d\plugify-module-v8.bat"

exit 0
