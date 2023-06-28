[win-hotcorners](https://github.com/matgat/win-hotcorners.git)
--------------------------------------------------------------

A rigid, spartan and light program, supposed to run in background,
that launches commands on mouse events in certain regions of screen.
inspired by [Tavis Ormandy](mailto:taviso@cmpxchg8b.com)'s
[*hotcorner*](https://github.com/taviso/hotcorner)
* Configurable with external links, no configuration file
* No graphic effects, lightweight process


Configure
---------
Set the path to the folder where you plan to put the links
of your actions either recompiling after editing the hard-coded
string literal defined in `ACTIONS_FOLDER` or just making a
*junction* to your existing folder:
```
> mklink /j  path\in\ACTIONS_FOLDER  path\to\existing-folder
```

Create there the links to your actions:
|                               |                               |
|-------------------------------|-------------------------------|
| `left-band-left-click.lnk`    | left band left click          |
| `left-band-middle-click.lnk`  | left band middle click        |
| `left-band-x-click.lnk`       | left band extended click      |
| `left-band-wheel-down.lnk`    | left band wheel down          |
| `left-band-wheel-up.lnk`      | left band wheel up            |
| `right-band-left-click.lnk`   | right band left click         |
| `right-band-middle-click.lnk` | right band middle click       |
| `right-band-right-click.lnk`  | right band left click         |
| `right-band-x-click.lnk`      | right band extended click     |
| `right-band-wheel-down.lnk`   | right band wheel down         |
| `right-band-wheel-up.lnk`     | right band wheel up           |
| `top-band-left-click.lnk`     | top band left click           |
| `top-band-x-click.lnk`        | top band extended click       |
| `top-band-wheel-down.lnk`     | top band wheel down           |
| `top-band-wheel-up.lnk`       | top band wheel up             |
| `top-right-left-click.lnk`    | top-right corner left click   |
| `top-right-middle-click.lnk`  | top-right corner middle click |
| `top-right-wheel-down.lnk`    | top-right corner wheel down   |
| `top-right-wheel-up.lnk`      | top-right corner wheel up     |

```
//                   [top]
//  [top-left] ┌─┬──════════──┬─┐ [top-right]
//             ├─┘            └─┤
//             ║                ║
//      [left] ║                ║ [right]
//             └────────────────┘
```
The top-left corner is not configurable and reserved to
launch the *Windows Task View* on hovering.


Build
-----
With [Microsoft Visual Studio](https://visualstudio.microsoft.com):
```
> git clone https://github.com/matgat/win-hotcorners.git
> msbuild msvc/hotcorners.vcxproj -t:hotcorners -p:Configuration=Release|x64
```

Install
-------
Windows binary is dynamically linked to Microsoft c++ runtime,
so needs the installation of
[`VC_redist.x64.exe`](https://aka.ms/vs/17/release/vc_redist.x64.exe)
as prerequisite.
Autostart the executable, for example creating a *shortcut* in `shell:startup`.


Uninstall
---------
Revert all your manual actions described above.
