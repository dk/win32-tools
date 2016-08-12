#include <windows.h>
#include <stdio.h>


void main( int argc, char ** argv)
{
   HWND hwnd = NULL, hwndAfter = NULL;
   int x, y, cx, cy;
   UINT flags;

   if ( argc < 8 && argc != 2) {
      printf("Format: SetWindowPos hwnd hwndAfter x y cx cy flags\n");
      printf("See detailed description in Win32 prog guide\n");
      printf("\nref table:\n");
      printf("-hwndAfter:\n");
      printf("  HWND_BOTTOM   : 1\n");
      printf("  HWND_NOTOPMOST:-2\n");
      printf("  HWND_TOP      : 0\n");
      printf("  HWND_TOPMOST  :-1\n");
      printf("-flags:\n");
      printf("  SWP_NOSIZE    : 0x0001\n");
      printf("  SWP_NOMOVE    : 0x0002\n");
      printf("  SWP_NOZORDER  : 0x0004\n");
      printf("  SWP_NOREDRAW  : 0x0008\n");
      printf("  SWP_NOACTIVATE: 0x0010\n");
      printf("  SWP_SHOWWINDOW: 0x0040\n");
      printf("  SWP_HIDEWINDOW: 0x0080\n");
      printf("\nsend bugs to <dmitry@karasik.eu.org>\n");
      exit(0);
   }

   if ( strcmp(argv[1], "NULL") != 0)
      hwnd      = ( HWND) strtoul( argv[1], NULL, 0);

   if ( argc == 2) {
      RECT r;
      if ( GetWindowRect( hwnd, &r))
         printf("origin: %d %d size: %d %d\n",
             r. left, r. top, r. right - r. left, r. bottom - r. top
         ); else printf( "SetWindowPos: %08x is not a window\n", hwnd);
      exit(0);
   }

   if ( strcmp(argv[2], "NULL") != 0)
      hwndAfter = ( HWND) strtoul( argv[2], NULL, 0);
   x         = ( int ) strtol( argv[3], NULL, 0);
   y         = ( int ) strtol( argv[4], NULL, 0);
   cx        = ( int ) strtol( argv[5], NULL, 0);
   cy        = ( int ) strtol( argv[6], NULL, 0);
   flags     = ( int ) strtoul( argv[7], NULL, 0);

   if ( !IsWindow( hwnd)) {
      fprintf( stderr, "SetWindowPos: %08x is not a window\n", hwnd);
      exit(0);
   }

   if ( !SetWindowPos( hwnd, hwndAfter, x, y, cx, cy, flags)) {
      fprintf( stderr, "SetWindowPos: error %d for HWND==%08x\n", GetLastError(), hwnd);
      exit(0);
   }

   if (( flags & ( SWP_NOMOVE|SWP_NOSIZE)) == 0) {
      RECT r;
      GetWindowRect( hwnd, &r);
      printf("New rect: %d %d %d %d", r.left, r.top, r.right, r.bottom);
   }
}
