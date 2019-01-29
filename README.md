# [win-hotcorners](https://matgat@bitbucket.org/matgat/win-hotcorners)

A rigid, spartan and light program, supposed to run in background, to add hotcorners functionality to windows 10, inspired by [Tavis Ormandy](mailto:taviso@cmpxchg8b.com)'s [*hotcorner*](https://github.com/taviso/hotcorner)

* Rigid: heavily optimized by hard-coded settings (no configuration file, settings change needs recompilation)
* Spartan: no GUI configurator or graphic effects
* Lightweight: should be nice with CPU usage, takes advantage of costexpr


## Build

* With *Visual Studio 2017*
    1. Open `vs2017\hotcorners.vcxproj`
    2. Select `release x64` and build project

* Compile directly?
    1. Start the command line with proper environment (*x64 Native Tools Command Prompt for VS 2017*) aka
    `%comspec% /k "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"`

    2. Compilation arguments:
        ```
        /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Fd"x64\Release\vc141.pdb" /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /std:c++latest /FC /Fa"x64\Release\" /EHsc /nologo /Fo"x64\Release\" /Fp"x64\Release\hotcorners.pch" /diagnostics:classic
        ```


## Installation
Customize your settings, build and move the `exe` (or create a *shortcut*) in `shell:startup`


## Uninstall
Just delete the executable
