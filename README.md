# [win-hotcorners](https://github.com/matgat/win-hotcorners.git)

A rigid, spartan and light program, supposed to run in background,
that launches commands on mouse events in certain regions of screen.
inspired by [Tavis Ormandy](mailto:taviso@cmpxchg8b.com)'s [*hotcorner*](https://github.com/taviso/hotcorner)

* Rigid: no configuration file, hard-coded settings
* Spartan: no GUI configurator or graphic effects
* Lightweight: should be nice with CPU usage, takes advantage of costexpr


## Build
* With _Visual Studio_
    1. Open `msvc\hotcorners.vcxproj`

* Compile directly?
    1. Start the command line with proper environment (*x64 Native Tools Command Prompt for VS*) aka
    `%comspec% /k "...\Microsoft Visual Studio\...\vcvars64.bat"`

    2. Compilation arguments:
        ```
        /permissive- /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Fd"x64\Release\vc141.pdb" /Zc:inline /fp:precise /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /std:c++latest /FC /Fa"x64\Release\" /EHsc /nologo /Fo"x64\Release\" /Fp"x64\Release\hotcorners.pch" /diagnostics:classic
        ```

## Configure
The predefined commands are links located in `%UserProfile%\sys\corner-actions`:
* `l-left-click.lnk`     (left band left click)
* `l-middle-click.lnk`   (left band middle click)
* `l-x-click.lnk`        (left band ext click)
* `l-wheel-down.lnk`     (left band wheel down)
* `l-wheel-up.lnk`       (left band wheel up)
* `r-left-click.lnk`     (right band left click)
* `r-middle-click.lnk`   (right band middle click)
* `r-right-click.lnk`    (right band left click)
* `r-x-click.lnk`        (right band ext click)
* `r-wheel-down.lnk`     (right band wheel down)
* `r-wheel-up.lnk`       (right band wheel up)
* `t-left-click.lnk`     (top band left click)
* `t-x-click.lnk`        (top band ext click)
* `t-wheel-down.lnk`     (top band wheel down)
* `t-wheel-up.lnk`       (top band wheel up)
* `tr-left-click.lnk`    (top-right corner left click)
* `tr-middle-click.lnk`  (top-right corner middle click)
* `tr-wheel-down.lnk`    (top-right corner wheel down)
* `tr-wheel-up.lnk`      (top-right corner wheel up)


## Install
Move the `exe` (or its *shortcut*) in `shell:startup`


## Uninstall
Just delete the executable
