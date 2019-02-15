/*  ---------------------------------------------
    hotcorners.cpp
    ©2019 matteo.gattanini@gmail.com

    OVERVIEW
    ---------------------------------------------
    Installs a mouse hook to trigger actions
    on certain mouse events.
    Inspired by: Tavis Ormandy <taviso@cmpxchg8b.com>
    https://github.com/taviso/hotcorner

    BUILD
    ---------------------------------------------
    MSVC2017: /O2 /std:c++17
    gcc: -O3 -std=c++17

    RELEASE HISTORY
    ---------------------------------------------
    (2009-01) First draft

    LICENSES
    ---------------------------------------------
    Use and modify freely

    USAGE:
    ---------------------------------------------
    No configuration file, map the events in
    'mouseHookCallback' function and compile
    Top left corner is fixed to "task manage"

                       [top]
    [top-left] ┌─┬────════════────┬─┐ [top-right]
               ├─┘                └─┤
               ║                    ║
       [left]  ║                    ║ [right]
               │                    │
               └────────────────────┘
    --------------------------------------------- */
#define WIN32_LEAN_AND_MEAN
// Standard C++ lib headers
#include <string_view>
#include <array>
#include <cstdint> // 'int16_t'
// MS Windows stuff
#include <windows.h>
#include <shellapi.h> // 'ShellExecuteEx'

#pragma comment(lib, "USER32")
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

// Just a debug facility
#ifdef _DEBUG
  #include <sstream>
  #define EVTLOG(msg) { std::wstringstream ss; ss << msg << '\n'; OutputDebugStringW(ss.str().c_str()); }
#else
  #define EVTLOG(x) ;
#endif


// Settings
constexpr LONG corner_size = 5; // [pix] Corner area size
constexpr DWORD dwell_time = 300; // [ms] Cursor dwell time for auto-trigger

// Actions to trigger: instead of reading from a config file,
// with compile time data we should avoid needless tests
using action_t = std::basic_string_view<TCHAR>;

// [top-left]
// mouse-dwell: <tasks> (WIN+TAB)

// [top-right]
constexpr action_t Rtr_lclick("A:\\Apps\\DwordBuilder.exe");
constexpr action_t Rtr_mclick(""); // A:\\Apps\\FileTypesMan.exe
constexpr action_t Rtr_rclick("");
constexpr action_t Rtr_xclick("");
constexpr action_t Rtr_wheelfwd("%windir%\\system32\\SnippingTool.exe");
constexpr action_t Rtr_wheelbck("%windir%\\system32\\SnippingTool.exe");

// [right]
constexpr action_t Rr_lclick("%UserProfile%\\Macotec");
constexpr action_t Rr_mclick("%UserProfile%\\Macotec\\Machines");
constexpr action_t Rr_rclick("%UserProfile%\\Macotec\\Macotec-Documents");
constexpr action_t Rr_xclick("%UserProfile%\\Macotec\\Devel-Bcb");
constexpr action_t Rr_wheelfwd("%UserProfile%\\Macotec\\Machines\\m32-Float");
constexpr action_t Rr_wheelbck("%UserProfile%\\Macotec\\Machines\\m32-Strato");

// [left]
constexpr action_t Rl_lclick("%UserProfile%");
constexpr action_t Rl_mclick("%UserProfile%\\Dev");
constexpr action_t Rl_rclick(""); // %UserProfile%\\Bin
constexpr action_t Rl_xclick("%UserProfile%\\Sys");
constexpr action_t Rl_wheelfwd(""); // %UserProfile%\\Dev
constexpr action_t Rl_wheelbck(""); // %UserProfile%\\Dev

// [top]
constexpr action_t Rt_lclick("%windir%\\system32\\calc.exe"); // /K \"cd %UserProfile%\"
constexpr action_t Rt_mclick("%windir%\\system32\\charmap.exe");
constexpr action_t Rt_rclick("");
constexpr action_t Rt_xclick("A:\\Apps\\FileTypesMan.exe");
constexpr action_t Rt_wheelfwd("%SystemRoot%\\system32\\WindowsPowerShell\\v1.0\\powershell.exe"); // %HOMEDRIVE%%HOMEPATH%
constexpr action_t Rt_wheelbck("%windir%\\system32\\cmd.exe");  // /K \"cd %UserProfile%\"


// Keyboard inputs
constexpr std::array<INPUT,4> inWinTab = {{// Win+Tab for task managing
                                            { INPUT_KEYBOARD, { VK_LWIN, 0 } },
                                            { INPUT_KEYBOARD, { VK_TAB,  0 } },
                                            { INPUT_KEYBOARD, { VK_TAB,  KEYEVENTF_KEYUP } },
                                            { INPUT_KEYBOARD, { VK_LWIN, KEYEVENTF_KEYUP } }
                                         }};

//---------------------------------------------------------------------------
inline bool in_rect(const RECT& r, const POINT& p) noexcept
   {
    return p.x>=r.left && p.x<=r.right && p.y>=r.top && p.y<r.bottom;
   }


//---------------------------------------------------------------------------
// Extract high word as signed
inline int16_t hi_word(const DWORD dw) noexcept
   {
    //#if sizeof(int16_t)!=2
    //    #error "what?? int16_t is not 2 bytes??"
    //#endif
    return int16_t((dw>>16) & 0xFFFF);
   }


//---------------------------------------------------------------------------
// Extract most significant bit from byte
inline bool msb(const BYTE b) noexcept { return (b & 0x80); }


//---------------------------------------------------------------------------
// Start a program/Open file (without wait)
void shell_exec(LPCTSTR pth) noexcept
{
    // Expand possible environmental variables
    std::array<TCHAR,MAX_PATH> buf;
    // Note: cls_DynArray always allocates a hidden null element in the end
    DWORD len = ::ExpandEnvironmentStrings(pth, buf.data(), (DWORD) buf.size());

    SHELLEXECUTEINFO ShExecInfo = {0};
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = 0; /* SEE_MASK_DEFAULT */
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL; // NULL=="open"
    ShExecInfo.lpFile = buf.data(); // TCHAR[MAX_PATH]
    ShExecInfo.lpParameters = NULL;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOWNORMAL; // SW_SHOW, SW_SHOWNA
    ShExecInfo.hInstApp = NULL;
    ::ShellExecuteEx(&ShExecInfo);

    // . Check errors
    //int retcode = reinterpret_cast<int>(H);
    //if( retcode >= 0 && retcode <= 32 )
    //   {
    //    switch( retcode )
    //       {
    //        case 0 : throw Exception("The operating system is out of memory or resources");
    //        case ERROR_FILE_NOT_FOUND : throw Exception("The specified file was not found");
    //        case ERROR_PATH_NOT_FOUND : throw Exception("The specified path was not found");
    //        case ERROR_BAD_FORMAT : throw Exception("The .EXE file is invalid (non-Win32 .EXE or error in .EXE image)");
    //        case SE_ERR_ACCESSDENIED : throw Exception("The operating system denied access to the specified file");
    //        case SE_ERR_ASSOCINCOMPLETE : throw Exception("The filename association is incomplete or invalid");
    //        case SE_ERR_DDEBUSY : throw Exception("The DDE transaction could not be completed because other DDE transactions were being processed");
    //        case SE_ERR_DDEFAIL : throw Exception("The DDE transaction failed");
    //        case SE_ERR_DDETIMEOUT : throw Exception("The DDE transaction could not be completed because the request timed out");
    //        case SE_ERR_DLLNOTFOUND : throw Exception("The specified dynamic-link library was not found");
    //        //case SE_ERR_FNF : throw Exception("The specified file was not found");
    //        case SE_ERR_NOASSOC : throw Exception("There is no application associated with the given filename extension");
    //        case SE_ERR_OOM : throw Exception("There was not enough memory to complete the operation");
    //        //case SE_ERR_PNF : throw Exception("The specified path was not found.");
    //        case SE_ERR_SHARE : throw Exception("A sharing violation occurred");
    //        default : throw Exception("An unknown error occurred");
    //       }
    //   }
} // 'shell_exec'


//---------------------------------------------------------------------------
// Get primary monitor rectangle
RECT get_screen_rect() noexcept
{
    // The bounding rectangle of all display monitors:
    //RECT Rscreen; ::GetWindowRect(::GetDesktopWindow(), &Rscreen);
    //int xo = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
    //int yo = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    //RECT Rscreen{xo, yo, xo+::GetSystemMetrics(SM_CXVIRTUALSCREEN), yo+::GetSystemMetrics(SM_CYVIRTUALSCREEN)};

    // Just the primary monitor (by definition has its upper left corner at (0,0))
    RECT Rscreen{ 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) };

    //HMONITOR h_pri_mon = ::MonitorFromPoint(POINT{0,0}, MONITOR_DEFAULTTOPRIMARY);
    //MONITORINFO mon_info{ 0 }; mon_info.cbSize = sizeof(mon_info);
    //::GetMonitorInfo(h_pri_mon, &mon_info);
    //RECT Rscreen{mon_info.rcMonitor};

    return Rscreen;
} // 'get_screen_rect'


//---------------------------------------------------------------------------
// Set zones on primary monitor
RECT Rtl, Rtr, Rr, Rl, Rt;
void set_zones(const LONG siz) noexcept
{
    RECT Rscreen = get_screen_rect();
    LONG Dw = (Rscreen.right - Rscreen.left) / 3; // [pix] Screeen width fraction
    LONG Dh = (Rscreen.bottom - Rscreen.top) / 3; // [pix] Screeen height fraction

    // Top left corner (autotrigger)
    constexpr LONG outside_size = 100; // [pix] Outside screen
                                       // This outside area is important because when moving
                                       // the cursor against the screen side its coordinates
                                       // are not limited but follow the mouse movement
    Rtl.left = Rscreen.left - outside_size;
    Rtl.top = Rscreen.top - outside_size;
    Rtl.right = Rscreen.left + siz;
    Rtl.bottom = Rscreen.top + siz;

    // Top right corner
    Rtr.left = Rscreen.right - siz;
    Rtr.top = Rscreen.top - 1;
    Rtr.right = Rscreen.right + 1;
    Rtr.bottom = Rscreen.top + siz;

    // Right band
    Rr.left = Rscreen.right - 1;
    Rr.top = Rscreen.top + Dh;
    Rr.right = Rscreen.right + 1;
    Rr.bottom = Rscreen.bottom - Dh;

    // Left band
    Rl.left = Rscreen.left - 1;
    Rl.top = Rr.top;
    Rl.right = Rscreen.left + 1;
    Rl.bottom = Rr.bottom;

    // Top band
    Rt.left = Rscreen.left + Dw;
    Rt.top = Rscreen.top - 1;
    Rt.right = Rscreen.right - Dw;
    Rt.bottom = Rscreen.top + 1;
} // 'set_zones'



//#include <windows.h>
//#include <gdiplus.h>
//#include <stdio.h>
//using namespace Gdiplus;
//
//void draw()
//{
//   // start up GDI+ -- only need to do this once per process at startup
//   GdiplusStartupInput gdiplusStartupInput;
//   ULONG_PTR gdiplusToken;
//   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//
//
//   Rect rect(20,20,50,50);
//   Graphics grpx(dc);
//   Image* image = new Image(L"SomePhoto.png");
//   grpx.DrawImage(Img,rect);
//
//   delete image;
//
//   // shut down - only once per process
//   GdiplusShutdown(gdiplusToken);
//   return;
//}


//---------------------------------------------------------------------------
// Check auto-trigger if the cursor stays in the zone without any activity
static DWORD WINAPI checkAutoTrigger(LPVOID lpParameter) noexcept
{
    const RECT* Rzone = reinterpret_cast<const RECT *>(lpParameter);

    ::Sleep( dwell_time ); // This should be nice for CPU load

    // No auto-trigger if any modifier keys or mouse button are pressed
    // Key is down if the most significant bit is 1
    BYTE ks[256]; // From winapi docs, the maximum keys are 256
    if( ::GetKeyboardState(ks) )
       {
        if( msb(ks[VK_SHIFT]) || msb(ks[VK_CONTROL]) || msb(ks[VK_MENU]) ||
            msb(ks[VK_LWIN]) || msb(ks[VK_RWIN]) ||
            msb(ks[VK_LBUTTON]) || msb(ks[VK_RBUTTON]) ) return -4;
       }

    // No auto-trigger if the cursor left the zone
    POINT cur_pos;
    if( !::GetCursorPos(&cur_pos) ) return -3;
    if( !in_rect(*Rzone, cur_pos) ) return -2;

    // Trigger the action
    // TODO: action basing on zone
    if( ::SendInput( (UINT) inWinTab.size(), (LPINPUT) inWinTab.data(), sizeof(INPUT)) != inWinTab.size() ) return -1;
    return 0; // Autotrigger done
} // 'checkAutoTrigger'


//---------------------------------------------------------------------------
static LRESULT CALLBACK mouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) noexcept
{
    if(nCode<0) return ::CallNextHookEx(NULL, nCode, wParam, lParam); // Do not process

    // Mouse message data
    MSLLHOOKSTRUCT *evt = (MSLLHOOKSTRUCT *) lParam;
    //    POINT     pt          // Coordinates
    //    DWORD     mouseData   // WM_MOUSEWHEEL: hiword>0:wheel-fwd, hiword<0:wheel-bck
    //                          // WM_*BUTTON*: hiword=1:btn1, hiword=2:btn2

    // Zones with autotrigger: check if mouse moved
    if( wParam==WM_MOUSEMOVE )
       {// 'Mouse moved'

        // ----------------------------- "top left" zone
        // Top left corner is fixed: lingering with the mouse, triggers <WIN+TAB>
        static HANDLE thrdRtl = INVALID_HANDLE_VALUE; // Thread checking "top left" zone autotrigger
        if( in_rect(Rtl, evt->pt) )
           {// In "top left"
            // Start auto-trigger check thread (if not already started)
            if( thrdRtl == INVALID_HANDLE_VALUE )
               {
                EVTLOG("Entered top left corner " << evt->pt.x << ';' << evt->pt.y)
                thrdRtl = ::CreateThread( nullptr,          // LPSECURITY_ATTRIBUTES   lpThreadAttributes
                                          0,                // SIZE_T                  dwStackSize
                                          checkAutoTrigger, // LPTHREAD_START_ROUTINE  lpStartAddress
                                          (LPVOID) &Rtl,    // LPVOID                  lpParameter
                                          0,                // DWORD                   dwCreationFlags
                                          nullptr );        // LPDWORD                 lpThreadId
               }
           }
        else
           {// Not in "top left"
            //EVTLOG("Point " << evt->pt.x << ';' << evt->pt.y << " not in " << Rtl.left << ';' << Rtl.top << " , " << Rtl.right << ';' << Rtl.bottom )
            // Abort auto-trigger check thread (if running)
            if( thrdRtl != INVALID_HANDLE_VALUE )
               {
                EVTLOG("Exited top left corner " << evt->pt.x << ';' << evt->pt.y)
                ::TerminateThread(thrdRtl, 0);
                ::CloseHandle(thrdRtl);
                thrdRtl = INVALID_HANDLE_VALUE;
               }
           }
       } // 'Mouse moved'

    // Zones that trigger with an action: check other mouse events
    else
       {// 'Other mouse event'
        //EVTLOG("WM_?? 0x" << std::hex << wParam)
        
        // Compile-time facilities: this should avoid needless tests
        constexpr bool Rtr_has_wheel_actions = Rtr_wheelfwd.size()>0 && Rtr_wheelbck.size()>0;
        constexpr bool Rtr_has_actions = Rtr_lclick.size()>0 || Rtr_mclick.size()>0 || Rtr_rclick.size()>0 || Rtr_xclick.size()>0 || Rtr_has_wheel_actions;
        constexpr bool Rr_has_wheel_actions = Rr_wheelfwd.size()>0 && Rr_wheelbck.size()>0;
        constexpr bool Rr_has_actions = Rr_lclick.size()>0 || Rr_mclick.size()>0 || Rr_rclick.size()>0 || Rr_xclick.size()>0 || Rr_has_wheel_actions;
        constexpr bool Rl_has_wheel_actions = Rl_wheelfwd.size()>0 && Rl_wheelbck.size()>0;
        constexpr bool Rl_has_actions = Rl_lclick.size()>0 || Rl_mclick.size()>0 || Rl_rclick.size()>0 || Rl_xclick.size()>0 || Rl_has_wheel_actions;
        constexpr bool Rt_has_wheel_actions = Rt_wheelfwd.size()>0 && Rt_wheelbck.size()>0;
        constexpr bool Rt_has_actions = Rt_lclick.size()>0 || Rt_mclick.size()>0 || Rt_rclick.size()>0 || Rt_xclick.size()>0 || Rt_has_wheel_actions;

        // ----------------------------- "top right" zone
        if( Rtr_has_actions && in_rect(Rtr, evt->pt) )
           {
            // 'Left click'
            if( Rtr_lclick.size()>0 && wParam==WM_LBUTTONDOWN )
               {
                EVTLOG("Left click in top right corner " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rtr_lclick.data() );
               }
            // 'Right click'
            else if( Rtr_rclick.size()>0 && wParam==WM_RBUTTONDOWN )
               {
                EVTLOG("Right click in top right corner " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rtr_rclick.data() );
               }
            // 'Middle click'
            else if( Rtr_mclick.size()>0 && wParam==WM_MBUTTONDOWN )
               {
                EVTLOG("Middle click in top right corner " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rtr_mclick.data() );
               }
            // 'eXpansion button click'
            else if( Rtr_xclick.size()>0 && wParam==WM_XBUTTONDOWN )
               {
                EVTLOG("X click in top right corner " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rtr_xclick.data() );
               }
            // 'Mouse wheel'
            else if( Rtr_has_wheel_actions && wParam==WM_MOUSEWHEEL )
               {
                if( Rtr_wheelfwd.size()>0 && hi_word(evt->mouseData)>0 )
                   {
                    EVTLOG("Fwd Wheel in top right corner " << hi_word(evt->mouseData))
                    shell_exec( Rtr_wheelfwd.data() );
                   }
                else if( Rtr_wheelbck.size() && hi_word(evt->mouseData)<0 )
                   {
                    EVTLOG("Bck Wheel in top right corner " << hi_word(evt->mouseData))
                    shell_exec( Rtr_wheelbck.data() );
                   }
               }
           } // "top right" zone

        // ----------------------------- "right band" zone
        else if( Rr_has_actions && in_rect(Rr, evt->pt) )
           {
            // 'Left click'
            if( Rr_lclick.size()>0 && wParam==WM_LBUTTONDOWN )
               {
                EVTLOG("Left click in right band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rr_lclick.data() );
               }
            // 'Right click'
            else if( Rr_rclick.size()>0 && wParam==WM_RBUTTONDOWN )
               {
                EVTLOG("Right click in right band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rr_rclick.data() );
               }
            // 'Middle click'
            else if( Rr_mclick.size()>0 && wParam==WM_MBUTTONDOWN )
               {
                EVTLOG("Middle click in right band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rr_mclick.data() );
               }
            // 'eXpansion button click'
            else if( Rr_xclick.size()>0 && wParam==WM_XBUTTONDOWN )
               {
                EVTLOG("X click in right band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rr_xclick.data() );
               }
            // 'Mouse wheel'
            else if( Rr_has_wheel_actions && wParam==WM_MOUSEWHEEL )
               {
                if( Rr_wheelfwd.size()>0 && hi_word(evt->mouseData)>0 )
                   {
                    EVTLOG("Fwd Wheel in right band " << hi_word(evt->mouseData))
                    shell_exec( Rr_wheelfwd.data() );
                   }
                else if( Rr_wheelbck.size() && hi_word(evt->mouseData)<0 )
                   {
                    EVTLOG("Bck Wheel in right band " << hi_word(evt->mouseData))
                    shell_exec( Rr_wheelbck.data() );
                   }
               }
           } // "right band" zone

        // ----------------------------- "left band" zone
        else if( Rl_has_actions && in_rect(Rl, evt->pt) )
           {
            // 'Left click'
            if( Rl_lclick.size()>0 && wParam==WM_LBUTTONDOWN )
               {
                EVTLOG("Left click in left band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rl_lclick.data() );
               }
            // 'Right click'
            else if( Rl_rclick.size()>0 && wParam==WM_RBUTTONDOWN )
               {
                EVTLOG("Right click in left band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rl_rclick.data() );
               }
            // 'Middle click'
            else if( Rl_mclick.size()>0 && wParam==WM_MBUTTONDOWN )
               {
                EVTLOG("Middle click in left band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rl_mclick.data() );
               }
            // 'eXpansion button click'
            else if( Rl_xclick.size()>0 && wParam==WM_XBUTTONDOWN )
               {
                EVTLOG("X click in left band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rl_xclick.data() );
               }
            // 'Mouse wheel'
            else if( Rl_has_wheel_actions && wParam==WM_MOUSEWHEEL )
               {
                if( Rl_wheelfwd.size()>0 && hi_word(evt->mouseData)>0 )
                   {
                    EVTLOG("Fwd Wheel in left band " << hi_word(evt->mouseData))
                    shell_exec( Rl_wheelfwd.data() );
                   }
                else if( Rl_wheelbck.size() && hi_word(evt->mouseData)<0 )
                   {
                    EVTLOG("Bck Wheel in left band " << hi_word(evt->mouseData))
                    shell_exec( Rl_wheelbck.data() );
                   }
               }
           } // "left band" zone

        // ----------------------------- "top band" zone
        else if( Rt_has_actions && in_rect(Rt, evt->pt) )
           {
            // 'Left click'
            if( Rt_lclick.size()>0 && wParam==WM_LBUTTONDOWN )
               {
                EVTLOG("Left click in top band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rt_lclick.data() );
               }
            // 'Right click'
            else if( Rt_rclick.size()>0 && wParam==WM_RBUTTONDOWN )
               {
                EVTLOG("Right click in top band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rt_rclick.data() );
               }
            // 'Middle click'
            else if( Rt_mclick.size()>0 && wParam==WM_MBUTTONDOWN )
               {
                EVTLOG("Middle click in top band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rt_mclick.data() );
               }
            // 'eXpansion button click'
            else if( Rt_xclick.size()>0 && wParam==WM_XBUTTONDOWN )
               {
                EVTLOG("X click in top band " << evt->pt.x << ';' << evt->pt.y)
                shell_exec( Rt_xclick.data() );
               }
            // 'Mouse wheel'
            else if( Rt_has_wheel_actions && wParam==WM_MOUSEWHEEL )
               {
                if( Rt_wheelfwd.size()>0 && hi_word(evt->mouseData)>0 )
                   {
                    EVTLOG("Fwd Wheel in top band " << hi_word(evt->mouseData))
                    shell_exec( Rt_wheelfwd.data() );
                   }
                else if( Rt_wheelbck.size() && hi_word(evt->mouseData)<0 )
                   {
                    EVTLOG("Bck Wheel in top band " << hi_word(evt->mouseData))
                    shell_exec( Rt_wheelbck.data() );
                   }
               }
           } // "top band" zone

       } // 'Other mouse event'

    // Finally, pass this event to possible other hooks
    return ::CallNextHookEx(NULL, nCode, wParam, lParam);
} // 'mouseHookCallback'


//---------------------------------------------------------------------------
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Init
    set_zones(corner_size);

    // Because ShellExecuteEx can delegate execution to Shell extensions (data sources, context menu handlers,
    // verb implementations) that are activated using Component Object Model (COM), COM should be initialized
    // before ShellExecuteEx is called. Some Shell extensions require the COM single-threaded apartment (STA)
    // type. In that case, COM should be initialized as shown here:
    //::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Intercept mouse events
    HHOOK mouse_hook;
    if( !(mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, mouseHookCallback, NULL, 0)) ) return 1;

    // Facility: key shortcut to quit this program
    //::RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT, 'Q'); // CTRL+ALT+Q

    // Message pump for 'WH_MOUSE_LL'
    MSG Msg;
    while( ::GetMessage(&Msg, NULL, 0, 0) )
       {
        //if(Msg.message == WM_HOTKEY) break; // Quit on hotkey
        ::DispatchMessage(&Msg);
       }

    // Finally
    ::UnhookWindowsHookEx(mouse_hook);
    return 0;
} // 'WinMain'
