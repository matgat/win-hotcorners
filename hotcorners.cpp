//  ---------------------------------------------
//  Windows program that installs a mouse hook to
//  trigger actions on certain mouse events
//  (ex. click a screen zone).
//  Inspired by: Tavis Ormandy <taviso@cmpxchg8b.com>
//  https://github.com/taviso/hotcorner
//  ---------------------------------------------
//  No configuration file, map the events with
//  links in ACTIONS_FOLDER defined below
//  Top left corner is fixed to "task manage"
//  ---------------------------------------------
//                   [top]
//  [top-left] ┌─┬──════════──┬─┐ [top-right]
//             ├─┘            └─┤
//             ║                ║
//      [left] ║                ║ [right]
//             └────────────────┘
//  ---------------------------------------------
//  Settings
constexpr int corner_size = 5; // [pix] Corner area size
constexpr unsigned int dwell_time = 300; // [ms] Cursor dwell time for auto-trigger
#define ACTIONS_FOLDER "C:\\Users\\user\\sys\\corner-actions\\"
//  ---------------------------------------------
// MS Windows stuff
#include <windows.h>
    // DWORD, RECT, POINT, LPVOID, LPSTR, HINSTANCE, ...
    // HANDLE, INVALID_HANDLE_VALUE, CloseHandle, CreateThread, TerminateThread
    // GetSystemMetrics, SM_CXSCREEN
    // HHOOK, MSLLHOOKSTRUCT, SetWindowsHookEx, CallNextHookEx, UnhookWindowsHookEx
    // MSG, LRESULT, CALLBACK, WPARAM, LPARAM, GetMessage, DispatchMessage, ...
    // WM_LBUTTONDOWN, ...
    // BYTE, GetKeyboardState, GetCursorPos, SendInput
#include <shellapi.h> // ShellExecuteEx, SHELLEXECUTEINFO
//#pragma comment(lib, "USER32")
//#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
// Note: On a '_WIN64' system, 'C:\Windows\System32\user32.dll' is a 64-bit library
//       The 32-bit version is in 'C:\Windows\SysWOW64\user32.dll'

#include <array> // std::array
#include <cstdint> // std::int16_t


    // Debug facility
  #ifdef _DEBUG
    #include <format> // std::format
    #define EVTLOG(s,...) ::OutputDebugString(std::format(s "\n",__VA_ARGS__).c_str());
  #else
    #define EVTLOG(...)
  #endif


/////////////////////////////////////////////////////////////////////////////
// My own mouse events ids for wheel-up and wheel-down
enum class mouse_event_t : char
   {
    left_button     = 'l',
    right_button    = 'r',
    middle_button   = 'm',
    extended_button = 'x',
    wheel_up        = '<',
    wheel_down      = '>',
    unknown         = '\0'
   };

//---------------------------------------------------------------------------
constexpr mouse_event_t get_event_id(const WPARAM win_mouse_event_id, const DWORD add_data) noexcept
{
    struct local
       {// Extract high word as signed
        [[nodiscard]] static inline constexpr std::int16_t hi_word(const DWORD dw) noexcept
           {
            static_assert( sizeof(std::int16_t)==2 );
            return std::int16_t((dw>>16) & 0xFFFF);
           }
       };

    switch( win_mouse_event_id )
       {
        case WM_LBUTTONDOWN: return mouse_event_t::left_button;
        case WM_RBUTTONDOWN: return mouse_event_t::right_button;
        case WM_MBUTTONDOWN: return mouse_event_t::middle_button;
        case WM_XBUTTONDOWN: return mouse_event_t::extended_button;
        case WM_MOUSEWHEEL:  return local::hi_word(add_data)>0 ? mouse_event_t::wheel_up
                                                               : mouse_event_t::wheel_down;
       }
    return mouse_event_t::unknown;
}


/////////////////////////////////////////////////////////////////////////////
class action_t final
{
 private:
    const char* m_command;

 public:

    constexpr action_t(const char* const cstr =nullptr) noexcept
      : m_command(cstr)
       {}

    [[nodiscard]] constexpr bool is_defined() const noexcept { return m_command!=nullptr; }

    //---------------------------------------------------------------------------
    void execute() const noexcept
       {
        if( is_defined() )
           {
            SHELLEXECUTEINFO ShExecInfo = {0};
            ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
            ShExecInfo.fMask = 0; /* SEE_MASK_DEFAULT */
            ShExecInfo.hwnd = NULL;
            ShExecInfo.lpVerb = NULL; // NULL=="open"
            ShExecInfo.lpFile = m_command;
            ShExecInfo.lpParameters = NULL;
            ShExecInfo.lpDirectory = NULL;
            ShExecInfo.nShow = SW_SHOWNORMAL; // SW_SHOW, SW_SHOWNA
            ShExecInfo.hInstApp = NULL;
            ::ShellExecuteEx(&ShExecInfo);
           }
       }
};


/////////////////////////////////////////////////////////////////////////////
struct event_action_pair_t final
  {
   mouse_event_t event;
   action_t action;
  };
template<std::size_t N> class actions_map_t
{
 private:
    event_action_pair_t elements[N];

 public:
    consteval actions_map_t(const event_action_pair_t(&arr)[N]) noexcept
       {
        for( std::size_t i=0; i<N; ++i )
           {
            elements[i].event = arr[i].event;
            elements[i].action = arr[i].action;
           }
       }

    [[nodiscard]] consteval action_t operator[](const mouse_event_t event) const noexcept
       {
        for( const event_action_pair_t& elem : elements )
           {
            if( elem.event==event )
               {
                return elem.action;
               }
           }
        return action_t{}; // Empty action
       }
};


// Defined as globals here because must be visible in mouseHookCallback
// Also, can't aggregate because rectangles are determined at runtime,
// while actions at compile time

RECT top_left_rect{};
//constexpr actions_map_t top_left_actions = <mouse-dwell: task view (WIN+TAB)>

RECT top_right_rect{};
constexpr actions_map_t top_right_actions =
  {{
    {mouse_event_t::left_button, ACTIONS_FOLDER "top-right-left-click.lnk"},
    {mouse_event_t::middle_button, ACTIONS_FOLDER "top-right-middle-click.lnk"},
    {mouse_event_t::wheel_up, ACTIONS_FOLDER "top-right-wheel-up.lnk"},
    {mouse_event_t::wheel_down, ACTIONS_FOLDER "top-right-wheel-down.lnk"}
  }};

RECT top_band_rect{};
constexpr actions_map_t top_band_actions =
  {{
    {mouse_event_t::left_button, ACTIONS_FOLDER "top-band-left-click.lnk"},
    {mouse_event_t::middle_button, ACTIONS_FOLDER "top-band-middle-click.lnk"},
    {mouse_event_t::extended_button, ACTIONS_FOLDER "top-band-x-click.lnk"},
    {mouse_event_t::wheel_up, ACTIONS_FOLDER "top-band-wheel-up.lnk"},
    {mouse_event_t::wheel_down, ACTIONS_FOLDER "top-band-wheel-down.lnk"}
  }};

RECT left_band_rect{};
constexpr actions_map_t left_band_actions =
  {{
    {mouse_event_t::left_button, ACTIONS_FOLDER "left-band-left-click.lnk"},
    {mouse_event_t::middle_button, ACTIONS_FOLDER "left-band-middle-click.lnk"},
    {mouse_event_t::extended_button, ACTIONS_FOLDER "left-band-x-click.lnk"},
    {mouse_event_t::wheel_up, ACTIONS_FOLDER "left-band-wheel-up.lnk"},
    {mouse_event_t::wheel_down, ACTIONS_FOLDER "left-band-wheel-down.lnk"}
  }};

RECT right_band_rect{};
constexpr actions_map_t right_band_actions =
  {{
    {mouse_event_t::left_button, ACTIONS_FOLDER "right-band-left-click.lnk"},
    {mouse_event_t::middle_button, ACTIONS_FOLDER "right-band-middle-click.lnk"},
    {mouse_event_t::right_button, ACTIONS_FOLDER "right-band-right-click.lnk"},
    {mouse_event_t::extended_button, ACTIONS_FOLDER "right-band-x-click.lnk"},
    {mouse_event_t::wheel_up, ACTIONS_FOLDER "right-band-wheel-up.lnk"},
    {mouse_event_t::wheel_down, ACTIONS_FOLDER "right-band-wheel-down.lnk"}
  }};


//---------------------------------------------------------------------------
void determine_screen_regions(const int size) noexcept
{
    // Retrieve the rectangle of the first monitor
    const RECT Rscreen{ 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) };

    //                   [top]
    //  [top-left] ┌─┬──════════──┬─┐ [top-right]
    //             ├─┘            └─┤
    //             ║                ║
    //      [left] ║                ║ [right]
    //             └────────────────┘

    top_left_rect =
        RECT{
                Rscreen.left - 100, // left
                Rscreen.top - 100, // top
                Rscreen.left + size, // right
                Rscreen.top + size // bottom
            };
    EVTLOG("top-left area: {};{} , {};{}", top_left_rect.left, top_left_rect.top, top_left_rect.right, top_left_rect.bottom)


    top_right_rect =
        RECT{
                Rscreen.right - size, // left
                Rscreen.top - 1, // top
                Rscreen.right + 1, // right
                Rscreen.top + size // bottom
            };
    EVTLOG("top-right area: {};{} , {};{}", top_right_rect.left, top_right_rect.top, top_right_rect.right, top_right_rect.bottom)

    top_band_rect =
        RECT{
                Rscreen.left + (Rscreen.right - Rscreen.left)/3, // left
                Rscreen.top - 1, // top
                Rscreen.right - (Rscreen.right - Rscreen.left)/3, // right
                Rscreen.top + 1 // bottom
            };
    EVTLOG("top-band area: {};{} , {};{}", top_band_rect.left, top_band_rect.top, top_band_rect.right, top_band_rect.bottom)

    left_band_rect =
        RECT{
                Rscreen.left - 1, // left
                Rscreen.top + (Rscreen.bottom - Rscreen.top)/3, // top
                Rscreen.left + 1, // right
                Rscreen.bottom - (Rscreen.bottom - Rscreen.top)/3 // bottom
            };
    EVTLOG("left-band area: {};{} , {};{}", left_band_rect.left, left_band_rect.top, left_band_rect.right, left_band_rect.bottom)

    right_band_rect =
        RECT{
                Rscreen.right - 1, // left
                Rscreen.top + (Rscreen.bottom - Rscreen.top)/3, // top
                Rscreen.right + 1, // right
                Rscreen.bottom - (Rscreen.bottom - Rscreen.top)/3 // bottom
            };
    EVTLOG("right-band area: {};{} , {};{}", right_band_rect.left, right_band_rect.top, right_band_rect.right, right_band_rect.bottom)
}


//---------------------------------------------------------------------------
[[nodiscard]] inline constexpr bool is_point_inside(const RECT& r, const POINT& p) noexcept
   {
    return p.x>=r.left && p.x<=r.right && p.y>=r.top && p.y<r.bottom;
   }


//---------------------------------------------------------------------------
[[nodiscard]] inline bool is_cursor_inside(const RECT& r) noexcept
   {
    POINT cur_pos;
    if( ::GetCursorPos(&cur_pos) )
       {
        return is_point_inside(r, cur_pos);
       }
    return false;
   }


//---------------------------------------------------------------------------
[[nodiscard]] inline bool is_modifier_key_pressed() noexcept
{
    struct local
       {// Key is down if the most significant bit is 1
        [[nodiscard]] static inline constexpr bool is_down(const BYTE b) noexcept { return (b & 0x80); }
       };

    BYTE keyboard_status[256]; // From winapi docs, the maximum keys are 256

    if( ::GetKeyboardState(keyboard_status) )
       {
        return local::is_down(keyboard_status[VK_SHIFT]) ||
               local::is_down(keyboard_status[VK_CONTROL]) ||
               local::is_down(keyboard_status[VK_MENU]) ||
               local::is_down(keyboard_status[VK_LWIN]) ||
               local::is_down(keyboard_status[VK_RWIN]) ||
               local::is_down(keyboard_status[VK_LBUTTON]) ||
               local::is_down(keyboard_status[VK_RBUTTON]);
       }
    return false;
}


//---------------------------------------------------------------------------
inline void activate_task_view() noexcept
{
    // Windows task view is activated by Win+Tab key sequence
    constexpr std::array<INPUT,4> inWinTab = {{
                                                { INPUT_KEYBOARD, { VK_LWIN, 0 } },
                                                { INPUT_KEYBOARD, { VK_TAB,  0 } },
                                                { INPUT_KEYBOARD, { VK_TAB,  KEYEVENTF_KEYUP } },
                                                { INPUT_KEYBOARD, { VK_LWIN, KEYEVENTF_KEYUP } }
                                             }};
    ::SendInput( (UINT) inWinTab.size(), (LPINPUT) inWinTab.data(), sizeof(INPUT) );
    //if( ret!=inWinTab.size() ) failed
}


//---------------------------------------------------------------------------
// Check auto-trigger if the cursor stays in the zone without any activity
static DWORD WINAPI check_autotrigger(LPVOID lpParameter) noexcept
{
    ::Sleep( dwell_time ); // This should be nice for CPU load

    // No auto-trigger if the cursor left the zone
    const RECT* const zone_rect = reinterpret_cast<const RECT *>(lpParameter);
    if( !is_cursor_inside(*zone_rect) )
       {
        return 1;
       }

    // No auto-trigger if any modifier keys or mouse button are pressed
    if( is_modifier_key_pressed() )
       {
        return 2;
       }

    activate_task_view();

    return 0;
}


//---------------------------------------------------------------------------
static LRESULT CALLBACK mouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam) noexcept
{
    if( nCode>=0 )
       {
        // Mouse message data
        MSLLHOOKSTRUCT* const evt = (MSLLHOOKSTRUCT*) lParam;
        //    DWORD     mouseData   // WM_MOUSEWHEEL: hiword>0:wheel-fwd, hiword<0:wheel-bck
        //                          // WM_*BUTTON*: hiword=1:btn1, hiword=2:btn2
        const POINT& cursor_pos = evt->pt;

        // Zones that trigger with an action: check other mouse events
        if( wParam!=WM_MOUSEMOVE )
           {//EVTLOG("WM_MOUSE* {:#06x}",wParam)
            const mouse_event_t event_id = get_event_id(wParam, evt->mouseData);

            #define SELECT_ACTION(ZONE)\
                switch( event_id )\
                   {\
                    case mouse_event_t::left_button: ZONE##_actions[mouse_event_t::left_button].execute(); break;\
                    case mouse_event_t::right_button: ZONE##_actions[mouse_event_t::right_button].execute(); break;\
                    case mouse_event_t::middle_button: ZONE##_actions[mouse_event_t::middle_button].execute(); break;\
                    case mouse_event_t::extended_button: ZONE##_actions[mouse_event_t::extended_button].execute(); break;\
                    case mouse_event_t::wheel_up: ZONE##_actions[mouse_event_t::wheel_up].execute(); break;\
                    case mouse_event_t::wheel_down: ZONE##_actions[mouse_event_t::wheel_down].execute(); break;\
                    case mouse_event_t::unknown: break;\
                   } // EVTLOG("Event {} in {} (cursor {};{})", event_id, #ZONE, cursor_pos.x, cursor_pos.y)

            if( is_point_inside(top_right_rect, cursor_pos) )
               {
                SELECT_ACTION(top_right)
               }
            else if( is_point_inside(top_band_rect, cursor_pos) )
               {
                SELECT_ACTION(top_band)
               }
            else if( is_point_inside(left_band_rect, cursor_pos) )
               {
                SELECT_ACTION(left_band)
               }
            else if( is_point_inside(right_band_rect, cursor_pos) )
               {
                SELECT_ACTION(right_band)
               }
           }

        // Zones with autotrigger: check if mouse moved
        else
           {// Mouse movement
            // Top left corner is fixed: lingering with the mouse, triggers task view
            static HANDLE h = INVALID_HANDLE_VALUE; // Thread checking "top left" zone autotrigger

            if( is_point_inside(top_left_rect, cursor_pos) )
               {// In "top left" area
                // Start auto-trigger check thread (if not already started)
                if( h==INVALID_HANDLE_VALUE )
                   {
                    EVTLOG("Entered top left corner {};{}", cursor_pos.x, cursor_pos.y)
                    h = ::CreateThread( nullptr,           // LPSECURITY_ATTRIBUTES   lpThreadAttributes
                                        0,                 // SIZE_T                  dwStackSize
                                        check_autotrigger, // LPTHREAD_START_ROUTINE  lpStartAddress
                                        (LPVOID) &top_left_rect, // LPVOID            lpParameter
                                        0,                 // DWORD                   dwCreationFlags
                                        nullptr );         // LPDWORD                 lpThreadId
                   }
               }
            else
               {// Not in "top left" area
                //EVTLOG("Cursor {};{} not in R[{};{},{};{}]", cursor_pos.x, cursor_pos.y, top_left_rect.left, top_left_rect.top, top_left_rect.right, top_left_rect.bottom)
                // Abort auto-trigger check thread (if running)
                if( h!=INVALID_HANDLE_VALUE )
                   {
                    EVTLOG("Exited top left corner {};{}", cursor_pos.x, cursor_pos.y)
                    ::TerminateThread(h, 0);
                    ::CloseHandle(h);
                    h = INVALID_HANDLE_VALUE;
                   }
               }
           }
       }

    // Finally, pass this event to possible other hooks
    return ::CallNextHookEx(NULL, nCode, wParam, lParam);
}


//---------------------------------------------------------------------------
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) //int main()
{
    EVTLOG("hotcorners " __DATE__ " " __TIME__)
    determine_screen_regions(corner_size);

    // Register a global key shortcut to quit this program?
    //::RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_ALT, 'Q'); // CTRL+ALT+Q

    // Intercept mouse events
    HHOOK mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, mouseHookCallback, NULL, 0);
    if( !mouse_hook  )
       {
        return 1;
       }

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
}



//#include <windows.h>
//#include <gdiplus.h>
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
