@REM Builds Visual Studio Solutions

cmake -H. -Bbuild -G "Visual Studio 16 2019" -A x64

@REM Compiles to exe

set VS2019TOOLS=    ^
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist %VS2019TOOLS% (
    echo VS 2019 Build Tools are missing!
    exit
)

call %VS2019TOOLS%

msbuild .\build\ALL_BUILD.vcxproj

exit