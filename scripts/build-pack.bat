@ECHO OFF

git --version > nul

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to get Git version, please check if it's installed.
    EXIT /B %ERRORLEVEL%
)

ECHO Cloning repository...
git clone --recursive https://github.com/cfnptr/pack pack

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to clone repository.
    EXIT /B %ERRORLEVEL%
)

CD pack
CALL build.bat

IF NOT %ERRORLEVEL% == 0 (
    EXIT /B %ERRORLEVEL%
)

CD build\Release
MOVE *.exe ..\..\.. > nul

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to move utilities.
    EXIT /B %ERRORLEVEL%
)

CD ..\..\..
RD /S /Q pack

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to remove repository directory.
    EXIT /B %ERRORLEVEL%
)

EXIT /B 0
