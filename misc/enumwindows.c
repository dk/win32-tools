#include <windows.h>
#include <stdio.h>


static int byname = 1;
static char buf[ 256];

BOOL get_window_string( HWND w)
{
   if ( byname)
      return GetWindowText( w, buf, 255);
   else
      return GetClassName( w, buf, 255);
}


BOOL CALLBACK ewpPS( HWND w, LPARAM dummy)
{
   if ( get_window_string( w))
      printf("%08x %s\n", w, buf);
   return TRUE;
}

int ok = 0;

BOOL CALLBACK ewp( HWND w, LPARAM str)
{
   if ( get_window_string( w) && ( strcmp((char*) str, buf) == 0)) {
      printf("%08x", w);
      ok = 1;
      return FALSE;
   }
   return TRUE;
}


void main( int argc, char ** argv)
{
   HWND wcc; 
   int enumerate = 1, bywindow = 0;
   char * enumerator = NULL;
   
   {
      int i;
      for ( i = 1; i < argc; i++) {
         char * c = argv[i];
         if ( *c != '-' || c[1] == '-') break;
         switch( c[1]) {
         case 'C':
            byname = 0;
            break;
         case 'c':
            {
               char * e;
               i++;
               wcc = ( HWND) strtoul( argv[i], &e, 0);
               if ( *e != '\0') {
                  printf("'%s' is not an integer\n", argv[i]);
                  exit(1);
               }
               bywindow = 1;
            }
            break;
         case 'h':
            printf("EnumWindows: enumerates window hierarchy\n"\
                   "call syntax:\n"\
                   "   EnumWindows [options]             - enumerates windows\n"\
                   "   EnumWindows [options] WindowName  - searches for a specified string\n"\
                   "options:\n"\
                   "   -c HWND - enumerates children of HWND\n"\
                   "   -C      - use window class instead of window name\n"\
                   "send bugs to <dmitry@karasik.eu.org>\n"
                   "");
            exit(0);
            break;
         }
      }
      if ( i + 1 < argc) {
         printf("Too many parameters\n");
         exit(1);
      } else if ( i < argc) {
         enumerator = argv[i];
         enumerate = 0;
      }
   }

   if ( enumerator) {
      if ( bywindow)
         EnumChildWindows( wcc, ewp, ( LPARAM) enumerator);
      else
         EnumWindows( ewp, ( LPARAM) enumerator);
      if ( !ok) printf("NULL");
   } else {
      if ( bywindow)
         EnumChildWindows( wcc, ewpPS, 0);
      else
         EnumWindows( ewpPS, 0);
   }

}
