[win-hotcorners](https://github.com/matgat/win-hotcorners.git)
--------------------------------------------------------------
[![ms-build](https://github.com/matgat/win-hotcorners/actions/workflows/ms-build.yml/badge.svg)](https://github.com/matgat/win-hotcorners/actions/workflows/ms-build.yml)

A rigid, spartan and light program, supposed to run in background,
that launches commands on mouse events in certain regions of screen.
inspired by [Tavis Ormandy](mailto:taviso@cmpxchg8b.com)'s
[*hotcorner*](https://github.com/taviso/hotcorner)

* Configurable with external links, compile-time configuration
* No graphic effects, lightweight process

-----
Usage
-----

1. Put the executable somewhere
2. Create in a folder the links to your actions (see table below)
   *default:* directory `hotcorners/` relative to executable
3. Launch the executable; generally you want to autostart it,
   for example creating a *shortcut* in `shell:startup`

> [!IMPORTANT]
> Windows binary is dynamically linked to Microsoft c++ runtime,
> so needs the installation of
> [`VC_redist.x64.exe`](https://aka.ms/vs/17/release/vc_redist.x64.exe)
> as prerequisite.

> [!TIP]
> To kill the process use the *task manager* or:
> ```batch
> $ Taskkill /IM "hotcorners.exe" /F
> ```


```
                     [top]
    [top-left] ┌─┬──════════──┬─┐ [top-right]
               ├─┘            └─┤
               ║                ║
        [left] ║                ║ [right]
               └────────────────┘
```

|                              |                                            |
|------------------------------|--------------------------------------------|
| `left-band-left-click.lnk`   | mouse left click on the left band          |
| `left-band-middle-click.lnk` | mouse middle click on the left band        |
| `left-band-x-click.lnk`      | mouse extended click on the left band      |
| `left-band-wheel-down.lnk`   | mouse wheel down on the left band          |
| `left-band-wheel-up.lnk`     | mouse wheel up on the left band            |
|------------------------------|--------------------------------------------|
| `right-band-left-click.lnk`  | mouse left click on the right band         |
| `right-band-middle-click.lnk`| mouse middle click on the right band       |
| `right-band-right-click.lnk` | mouse right click on the right band        |
| `right-band-x-click.lnk`     | mouse extended click on the right band     |
| `right-band-wheel-down.lnk`  | mouse wheel down on the right band         |
| `right-band-wheel-up.lnk`    | mouse wheel up on the right band           |
|------------------------------|--------------------------------------------|
| `top-band-left-click.lnk`    | mouse left click on the top band           |
| `top-band-x-click.lnk`       | mouse extended click on the top band       |
| `top-band-wheel-down.lnk`    | mouse wheel down on the top band           |
| `top-band-wheel-up.lnk`      | mouse wheel up on the top band             |
|------------------------------|--------------------------------------------|
| `top-right-left-click.lnk`   | mouse left click on the top-right corner   |
| `top-right-middle-click.lnk` | mouse middle click on the top-right corner |
| `top-right-wheel-down.lnk`   | mouse wheel down on the top-right corner   |
| `top-right-wheel-up.lnk`     | mouse wheel up on the top-right corner     |


> [!NOTE]
> * The top-left corner is not configurable and reserved to
>   launch the _Windows Task View_ on hovering
> * The bottom corners are reserved for start menu and show desktop
> * Right opens a context menu so it's exceptionally defined
>   just for the _right band_



---------
Configure
---------
Configuration parameter are hard coded (👀) in the source:

```cpp
constexpr int corner_size = 5; // [pix] Corner area size
constexpr unsigned int dwell_time = 300; // [ms] Cursor dwell time for auto-trigger
#define ACTIONS_FOLDER "hotcorners\\"
```

The folder where put the links of your actions is defined by
the string literal defined with `ACTIONS_FOLDER`.

If you want to change it you have three options:

* Edit the source and recompile
* Launch the program with the current directory set as the parent of
  your action folder (for example with a shortcut)
* Create a *junction* of the actual folder you plan to use:

```batch
> cd /d path\of\hotcorners-executable
> mklink /j  "hotcorners"  path\to\existing\folder
```

-----
Build
-----
With [Microsoft Visual Studio](https://visualstudio.microsoft.com):

```batch
> git clone https://github.com/matgat/win-hotcorners.git
> msbuild win-hotcorners/build/hotcorners.vcxproj -t:Rebuild -p:Configuration=Release -p:Platform=x64
> ./win-hotcorners/build/win-x64-Release/hotcorners.exe
```
