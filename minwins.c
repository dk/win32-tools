#define APPNAME "Minimized Windows Viewer"
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "list.h"

#define CM_TOGGLEIGN   (WM_USER+1)
#define CM_HELP        (WM_USER+2)
#define CM_TOGGLEVIS   0x7FF0
#define CM_CLOSE       0x7FFF
#define VS_VISIBLE     0
#define VS_HIDDEN      1
#define VS_ALL         2

#define CLASSNAME      "MINWINS_AEXP0!@#$%^&*("
#define APPICON        "APPICON"

static int  useIgnored  = 1;
static int  useCompact  = 0;
static int  viewState   = VS_ALL;

static int  windowsCount = 0;
static int  ignoredCount = 0;
static HWND windows[ 1024];
static HWND ignored[ 1024];
static char misc[ 256];
static List strings;

static void help(void) {
   MessageBox( NULL,
    "-s XXX  : initial state, could be VISIBLE, HIDDEN, ALL\n" \
    "-i      : if specified, ignored state is OFF\n" \
    "-x file : file contained permanently ignored windows\n" \
    "-compact: left-click popup shows no control items",
    "Parameters", MB_OK|MB_ICONINFORMATION);
}

BOOL CALLBACK ewp( HWND hwnd, HMENU m)
{
   int i;
   char buf[ 256];
   BOOL visible = IsWindowVisible( hwnd);

   if ( !visible && useIgnored) {
      int i;
      for ( i = 0; i < ignoredCount; i++)
         if ( ignored[ i] == hwnd) return TRUE;
   }

   switch ( viewState) {
   case VS_VISIBLE:
      if ( !visible) return TRUE;
      break;
   case VS_HIDDEN:
      if ( visible) return TRUE;
      break;
   }
   if ( !GetWindowText( hwnd, buf, 255))
      return TRUE;

   for ( i = 0; i < strings. count; i++)
      if ( strcmp( strings. items[i], buf) == 0)
         return TRUE;

   windows[ windowsCount] = hwnd;
   AppendMenu( m, MF_STRING | (( visible && ( viewState == VS_ALL)) ? MF_CHECKED : 0), windowsCount, buf);
   return windowsCount++ < 1024;
}

BOOL CALLBACK ewp_ignore( HWND hwnd, LPARAM dummy)
{
   if ( !IsWindowVisible( hwnd)) ignored[ ignoredCount] = hwnd;
   return ignoredCount++ < 1024;
}

static void
menus( HWND hWnd, BOOL doEnum)
{
   HMENU m;
   POINT lp;
   m = CreatePopupMenu();

   if ( doEnum) {
       windowsCount = 0;
       EnumWindows( ewp, ( LPARAM) m);
       if ( !useCompact)
          AppendMenu( m, MF_SEPARATOR, 0, NULL);
   } else {
       AppendMenu( m, MF_STRING | ( useIgnored ? MF_CHECKED : 0), CM_TOGGLEIGN, "&Ignore hidden");
       AppendMenu( m, MF_STRING, CM_HELP, "&Help");
       AppendMenu( m, MF_SEPARATOR, 0, NULL);
   }
   if ( !doEnum || !useCompact) {
      AppendMenu( m, MF_STRING | ( viewState == VS_VISIBLE ? MF_CHECKED : 0), CM_TOGGLEVIS + VS_VISIBLE, "View &visible");
      AppendMenu( m, MF_STRING | ( viewState == VS_HIDDEN  ? MF_CHECKED : 0), CM_TOGGLEVIS + VS_HIDDEN , "View &hidden");
      AppendMenu( m, MF_STRING | ( viewState == VS_ALL     ? MF_CHECKED : 0), CM_TOGGLEVIS + VS_ALL    , "View &all");
      AppendMenu( m, MF_STRING, CM_CLOSE,     "&Close");
   }
   GetCursorPos( &lp);
   SetForegroundWindow( hWnd);
   TrackPopupMenu( m, TPM_LEFTBUTTON|TPM_LEFTALIGN|TPM_RIGHTBUTTON, lp.x, lp.y, 0, hWnd, NULL);
   PostMessage( hWnd, WM_USER, 0, 0);
}


#define WM_NOTIFYICON (WM_USER+100)

static HWND      frame;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LRESULT     ret = 0;
   switch ( message) {
   case WM_DESTROY:
      PostQuitMessage(0);
      break;
   case WM_COMMAND:
      switch( wParam) {
      case CM_CLOSE:
         PostQuitMessage(0);
         break;
      case CM_TOGGLEVIS + VS_VISIBLE:
      case CM_TOGGLEVIS + VS_HIDDEN:
      case CM_TOGGLEVIS + VS_ALL:
         viewState = wParam - CM_TOGGLEVIS;
         break;
      case CM_TOGGLEIGN:
         ignoredCount = 0;
         useIgnored = !useIgnored;
         if ( useIgnored) EnumWindows( ewp_ignore, 0);
         break;
      case CM_HELP:
         help();
         break;
      default:
         if (( wParam >= 0) && ((int) wParam < windowsCount) && IsWindow( windows[ wParam]))
            SetWindowPos( windows[ wParam], 0, 0, 0, 0, 0,
                SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|
                ( IsWindowVisible(windows[ wParam]) ? ( SWP_HIDEWINDOW|SWP_NOACTIVATE) : SWP_SHOWWINDOW)
            );
      }
      return 0;
   case WM_NOTIFYICON:
      switch( lParam) {
      case WM_LBUTTONDOWN:
         menus( hWnd, 1);
         break;
      case WM_RBUTTONDOWN:
         menus( hWnd, 0);
         break;
      }
      return 0;
   }
   ret = DefWindowProc(hWnd, message, wParam, lParam);
   return ret;
}



static void die( char * msg)
{
   MessageBox( NULL, msg, "Error", MB_OK|MB_ICONHAND);
   exit(0);
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
   int argc = 1;
   char ** argv = malloc( 80 * sizeof( char*));

   memset( argv, 0, 80 * sizeof( char*));
   list_create( &strings, 8, 8);

   if ( FindWindow( CLASSNAME, NULL))return TRUE;

   {
      int quote = 0;
      char * cmd = ( char *) lpCmdLine;

      argv[0] = cmd;
      while ( *cmd) {
         if ( *cmd == '"') {
            if ( *(cmd+1) == '"') {
               memmove( cmd, cmd + 1, strlen( cmd + 1));
               cmd += 2;
               continue;
            }

            quote = !quote;
            if ( quote)
               memmove( cmd, cmd + 1, strlen( cmd + 1));
            else
               goto NEWARG;
         }
         if ( *cmd == ' ' && !quote) {
         NEWARG:
            argv[ argc++] = cmd + 1;
            *cmd = '\0';
            if ( argc == 79) break;
         }
         cmd++;
      }
   }

   {
      int i;
      for ( i = 0; i < argc; i++) {
         if (( stricmp( argv[i], "h") == 0) ||
             ( stricmp( argv[i], "?") == 0) ||
             ( stricmp( argv[i], "-?") == 0) ||
             ( stricmp( argv[i], "-h") == 0) ||
             ( stricmp( argv[i], "help") == 0) ||
             ( stricmp( argv[i], "-help") == 0)) {
            help();
            exit(0);
         }
         if ( strcmp( argv[i], "-s") == 0) {
            char * state = argv[i+1];

            if ( !state)
               die("-s needs state specifier, VISIBLE, HIDDEN or ALL");

            if ( stricmp( state, "visible") == 0)
               viewState = VS_VISIBLE;
            else if ( stricmp( state, "hidden") == 0)
               viewState = VS_HIDDEN;
            else if ( stricmp( state, "all") == 0)
               viewState = VS_ALL;
            else
               die("invalid -s state specifier, need to be VISIBLE, HIDDEN or ALL");
            i++;
         } else if ( strcmp( argv[i], "-i") == 0) {
            useIgnored = 0;
         } else if ( strcmp( argv[i], "-compact") == 0) {
            useCompact = 1;
         } else if ( strcmp( argv[i], "-x") == 0) {
            int f;
            char * fname = argv[i+1];
            char rdb;
            char buf[ 256];
            int cnt = 0;
            if ( !fname)
               die("-x needs filename");
            f = _open( fname, _O_TEXT | _O_RDONLY);
            if ( f == -1) {
               char buf[256];
               _snprintf( buf, 256, "Cannot open %s", fname);
               die( buf);
            }
            while ( _read( f, &rdb, 1) == 1) {
               if ( rdb == '\n') {
                  buf[ cnt] = '\0';
                  if ( cnt > 1)
                     list_add( &strings, strcpy( malloc( cnt), buf));
                  cnt = 0;
               } else
                  buf[ cnt++] = rdb;
            }
            if ( cnt > 1)
               list_add( &strings, strcpy( malloc( cnt + 1), buf));
            _close( f);
            i++;
         }
      }
   }

   {
       WNDCLASS  wc;

       memset( &wc, 0, sizeof( wc));
       wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
       wc.lpfnWndProc   = (WNDPROC)WndProc;
       wc.cbClsExtra    = 0;
       wc.cbWndExtra    = 0;
       wc.hInstance     = hInstance;
       wc.hIcon         = LoadIcon( hInstance, APPICON);
       wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
       wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
       wc.lpszClassName = CLASSNAME;
       wc.lpszMenuName  = NULL;
       RegisterClass( &wc);

       frame = CreateWindow( CLASSNAME, "Hidden Window Manager by Dmitry Karasik, dk@plab.ku.dk",
            WS_SYSMENU | WS_THICKFRAME| WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
            100, 100, 0, 0, NULL, NULL, hInstance, NULL);
   }

   {
      NOTIFYICONDATA pnid = {
         sizeof( NOTIFYICONDATA),
         frame,
         0,
         NIF_ICON|NIF_MESSAGE|NIF_TIP,
         WM_NOTIFYICON,
         LoadIcon( hInstance, APPICON),
         "Hidden Window Manager"
      };
      Shell_NotifyIcon( NIM_ADD, &pnid);
      if ( pnid. hIcon) DestroyIcon( pnid. hIcon);
   }

   if ( useIgnored) EnumWindows( ewp_ignore, 0);

   {
      MSG msg;
      while ( GetMessage(&msg, NULL, 0, 0)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }

   {
      NOTIFYICONDATA pnid = {
         sizeof( NOTIFYICONDATA),
         frame,
         0
      };
      Shell_NotifyIcon( NIM_DELETE, &pnid);
   }

   DestroyWindow( frame);
   frame = NULL;
   return TRUE;
}


