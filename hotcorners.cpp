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
inline static constexpr int corner_size = 5; // [pix] Corner area size
inline static constexpr unsigned int dwell_time = 300; // [ms] Cursor dwell time for auto-trigger
#define ACTIONS_FOLDER "hotcorners\\"
//  ---------------------------------------------
#include <windows.h>
#include <shellapi.h> // ShellExecuteEx, SHELLEXECUTEINFO
//  ---------------------------------------------
#include <array> // std::array
#include <cstdint> // std::int16_t


  #ifdef _DEBUG
    #include <format> // std::format
    #define DBGLOG(s,...) ::OutputDebugString(std::format(s "\n",__VA_ARGS__).c_str());
  #else
    #define DBGLOG(...)
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
    struct local final
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
template<std::size_t N> class actions_map_t final
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
//inline static constexpr actions_map_t top_left_actions = <mouse-dwell: task view (WIN+TAB)>

RECT top_right_rect{};
inline static constexpr actions_map_t top_right_actions =
  {{
    {mouse_event_t::left_button, ACTIONS_FOLDER "top-right-left-click.lnk"},
    {mouse_event_t::middle_button, ACTIONS_FOLDER "top-right-middle-click.lnk"},
    {mouse_event_t::wheel_up, ACTIONS_FOLDER "top-right-wheel-up.lnk"},
    {mouse_event_t::wheel_down, ACTIONS_FOLDER "top-right-wheel-down.lnk"}
  }};

RECT top_band_rect{};
inline static constexpr actions_map_t top_band_actions =
  {{
    {mouse_event_t::left_button, ACTIONS_FOLDER "top-band-left-click.lnk"},
    {mouse_event_t::middle_button, ACTIONS_FOLDER "top-band-middle-click.lnk"},
    {mouse_event_t::extended_button, ACTIONS_FOLDER "top-band-x-click.lnk"},
    {mouse_event_t::wheel_up, ACTIONS_FOLDER "top-band-wheel-up.lnk"},
    {mouse_event_t::wheel_down, ACTIONS_FOLDER "top-band-wheel-down.lnk"}
  }};

RECT left_band_rect{};
inline static constexpr actions_map_t left_band_actions =
  {{
    {mouse_event_t::left_button, ACTIONS_FOLDER "left-band-left-click.lnk"},
    {mouse_event_t::middle_button, ACTIONS_FOLDER "left-band-middle-click.lnk"},
    {mouse_event_t::extended_button, ACTIONS_FOLDER "left-band-x-click.lnk"},
    {mouse_event_t::wheel_up, ACTIONS_FOLDER "left-band-wheel-up.lnk"},
    {mouse_event_t::wheel_down, ACTIONS_FOLDER "left-band-wheel-down.lnk"}
  }};

RECT right_band_rect{};
inline static constexpr actions_map_t right_band_actions =
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
    DBGLOG("top-left area: {};{} , {};{}", top_left_rect.left, top_left_rect.top, top_left_rect.right, top_left_rect.bottom)


    top_right_rect =
        RECT{
                Rscreen.right - size, // left
                Rscreen.top - 1, // top
                Rscreen.right + 1, // right
                Rscreen.top + size // bottom
            };
    DBGLOG("top-right area: {};{} , {};{}", top_right_rect.left, top_right_rect.top, top_right_rect.right, top_right_rect.bottom)

    top_band_rect =
        RECT{
                Rscreen.left + (Rscreen.right - Rscreen.left)/3, // left
                Rscreen.top - 1, // top
                Rscreen.right - (Rscreen.right - Rscreen.left)/3, // right
                Rscreen.top + 1 // bottom
            };
    DBGLOG("top-band area: {};{} , {};{}", top_band_rect.left, top_band_rect.top, top_band_rect.right, top_band_rect.bottom)

    left_band_rect =
        RECT{
                Rscreen.left - 1, // left
                Rscreen.top + (Rscreen.bottom - Rscreen.top)/3, // top
                Rscreen.left + 1, // right
                Rscreen.bottom - (Rscreen.bottom - Rscreen.top)/3 // bottom
            };
    DBGLOG("left-band area: {};{} , {};{}", left_band_rect.left, left_band_rect.top, left_band_rect.right, left_band_rect.bottom)

    right_band_rect =
        RECT{
                Rscreen.right - 1, // left
                Rscreen.top + (Rscreen.bottom - Rscreen.top)/3, // top
                Rscreen.right + 1, // right
                Rscreen.bottom - (Rscreen.bottom - Rscreen.top)/3 // bottom
            };
    DBGLOG("right-band area: {};{} , {};{}", right_band_rect.left, right_band_rect.top, right_band_rect.right, right_band_rect.bottom)
}


//---------------------------------------------------------------------------
[[nodiscard]] inline constexpr bool is_point_inside(const RECT& r, const POINT& p) noexcept
   {
    return p.x>=r.left && p.x<=r.right && p.y>=r.top && p.y<r.bottom;
   }


//---------------------------------------------------------------------------
[[nodiscard]] inline bool is_modifier_key_pressed() noexcept
{
    struct local final
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
    // Windows task view is activated by Win+Tab key sequence:
    static constexpr std::array<INPUT,4> inWinTab =
      {{
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
static DWORD WINAPI check_autotrigger_in(LPVOID lpParameter) noexcept
{
    // The input parameter is a pointer to the RECT of the zone
    const RECT* const zone_rect = reinterpret_cast<const RECT *>(lpParameter);

    ::Sleep( dwell_time ); // This should be nice for CPU load

    // No auto-trigger if the cursor left the zone
    // I have to check the new position of the cursor
    struct local final
       {
        [[nodiscard]] static inline bool is_cursor_inside(const RECT& r) noexcept
           {
            POINT cur_pos;
            if( ::GetCursorPos(&cur_pos) )
               {
                return is_point_inside(r, cur_pos);
               }
            return false;
           }
       };
    if( !local::is_cursor_inside(*zone_rect) )
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

/////////////////////////////////////////////////////////////////////////////
class check_autotrigger_thread_t final
{
 private:
    HANDLE m_handle = INVALID_HANDLE_VALUE;

 public:
    ~check_autotrigger_thread_t() noexcept
       {
        if( running() ) stop();
       }

    [[nodiscard]] bool running() const noexcept { return m_handle!=INVALID_HANDLE_VALUE; }

    void start() noexcept
       {
        //                     sec-attrib stack   routine                   param          flags  id
        m_handle = ::CreateThread(nullptr, 0, check_autotrigger_in, (LPVOID) &top_left_rect, 0, nullptr);
       }

    void stop() noexcept
       {
        ::TerminateThread(m_handle, 0);
        ::CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
       }
};


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
           {//DBGLOG("WM_MOUSE* {:#06x}",wParam)
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
                   } // DBGLOG("Event {} in {} (cursor {};{})", event_id, #ZONE, cursor_pos.x, cursor_pos.y)

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
            static check_autotrigger_thread_t check_trigger; // "top left" zone autotrigger thread

            if( is_point_inside(top_left_rect, cursor_pos) )
               {
                if( not check_trigger.running() )
                   {
                    DBGLOG("Entered top left corner {};{}", cursor_pos.x, cursor_pos.y)
                    check_trigger.start();
                   }
               }
            else
               {
                if( check_trigger.running() )
                   {
                    DBGLOG("Exited top left corner {};{}", cursor_pos.x, cursor_pos.y)
                    check_trigger.stop();
                   }
               }
           }
       }

    // Pass this event to possible other hooks
    return ::CallNextHookEx(NULL, nCode, wParam, lParam);
}


//---------------------------------------------------------------------------
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) //int main()
{
    DBGLOG("hotcorners " __DATE__ " " __TIME__)
    determine_screen_regions(corner_size);

    // Intercept mouse events
    HHOOK mouse_hook = ::SetWindowsHookEx(WH_MOUSE_LL, mouseHookCallback, NULL, 0);
    if( !mouse_hook  )
       {
        return 1;
       }

    // Message pump for 'WH_MOUSE_LL'
    // Note: WM_QUIT will cause ::GetMessage to return NULL
    MSG msg;
    while( ::GetMessage(&msg, NULL, 0, 0) )
       {
        ::DispatchMessage(&msg);
       }

    // Finally
    ::UnhookWindowsHookEx(mouse_hook);
    return 0;
}


// Maybe in the future...
//#include <gdiplus.h>
//class GdiPlusEnv final
//{
// private:
//    ULONG_PTR m_token;
//
// public:
//    GdiPlusEnv() noexcept
//       {
//        Gdiplus::GdiplusStartupInput startup_input;
//        Gdiplus::GdiplusStartup(&m_token, &startup_input, NULL);
//       }
//
//    ~GdiPlusEnv() noexcept
//       {
//        Gdiplus::GdiplusShutdown(m_token);
//       }
//};
//int main()
//{
//   GdiPlusEnv env;
//
//   Gdiplus::Rect rect(20,20,50,50);
//   Gdiplus::Graphics grpx(dc);
//   Gdiplus::Image image("SomePhoto.png");
//   grpx.DrawImage(image,rect);
//}
