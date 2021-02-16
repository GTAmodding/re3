@echo off

REM creates version.h with HEAD commit hash
REM params: $1=full path to output file (usually points version.h)

setlocal enableextensions enabledelayedexpansion

cd /d "%~dp0"

break> %1

<nul set /p=^"#define GIT_SHA1 ^"^"> %1

where git
if "%errorlevel%" == "0" ( goto :havegit ) else ( goto :writeending )

:havegit
for /f %%v in ('git rev-parse --short HEAD') do set version=%%v
<nul set /p="%version%" >> %1

:writeending

echo ^" >> %1
echo const char* g_GIT_SHA1 = GIT_SHA1; >> %1

EXIT /B