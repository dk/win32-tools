#include <windows.h>
#include <stdio.h>


void main( int argc, char ** argv)
{
   HWND hwnd = NULL;
   BOOL ena;

   if ( argc < 2) {
      printf("Format: EnableWindow hwnd [enable]\n"\
             "See detailed description in Win32 prog guide\n"\
             "send bugs to <dmitry@karasik.eu.org>\n");
      exit(0);
   }

   if ( strcmp(argv[1], "NULL") != 0)
      hwnd      = ( HWND) strtoul( argv[1], NULL, 0);

   if ( !IsWindow( hwnd)) {
      fprintf( stderr, "EnableWindow: %08x is not a window\n", hwnd);
      exit(0);
   }

   if ( argc > 2) {
      ena = ( BOOL) strtoul( argv[2], NULL, 0);
      EnableWindow( hwnd, ena);
   } else
      printf( "Window is %s\n", IsWindowEnabled( hwnd) ? "enabled" : "disabled");
}
