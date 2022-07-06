@ECHO OFF

spirv-cross --revision > nul

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to get SPIRV-CROSS version, please check if Vulkan SDK is installed.
    EXIT /B %ERRORLEVEL%
)

MKDIR decompiled

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to create decompiled directory.
    EXIT /B %ERRORLEVEL%
)

ECHO Decompiling shaders...

FOR %%f IN (*.spv) DO (
    spirv-cross --version 460 --no-es --vulkan-semantics %%f --output _decompiled/%%f.txt

    IF %ERRORLEVEL% == 0 (
        ECHO Decompiled "%%f" shader.
    ) ELSE (
        ECHO Failed to decompile "%%f" shader.
        EXIT /B %ERRORLEVEL%
    )
)

EXIT /B 0
