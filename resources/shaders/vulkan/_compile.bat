@ECHO OFF

glslc --version

IF NOT %ERRORLEVEL% == 0 (
    ECHO Failed to get GLSLC version, please check if Vulkan SDK is installed.
    PAUSE
    EXIT
)

ECHO(
ECHO Compiling shaders...

FOR %%f IN (*.vert *.tesc *.tese *.geom *.frag *.comp *.rgen *.rahit *.rchit *.rmiss *.rint *.rcall *.task *.mesh) DO (
    glslc --target-env=vulkan1.2 -c -O %%f -o %%f.spv

    IF %ERRORLEVEL% == 0 (
        ECHO Compiled "%%f" shader.
    ) ELSE (
        ECHO Failed to compile "%%f" shader.
        PAUSE
        EXIT
    )
)

PAUSE
