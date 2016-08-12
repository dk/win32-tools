#include <windows.h>
#include <stdio.h>


int main( int argc, char ** argv)
{
   char p1[ 1024] = "";
   char p2[ 1024] = "";
   UINT flags = MB_OK;
   int ret = IDCANCEL;

   if ( argc < 2) {
      printf("Format: MessageBox message [ text [ flags]]\n");
      printf("See detailed description in Win32 prog guide\n");
      printf("\nref table:\n");
      printf("  MB_OK              : 0x00000000\n");
      printf("  MB_CANCEL          : 0x00000001\n");
      printf("  MB_ABORTRETRYIGNORE: 0x00000002\n");
      printf("  MB_YESNOCANCEL     : 0x00000003\n");
      printf("  MB_YESNO           : 0x00000004\n");
      printf("  MB_RETRYCANCEL     : 0x00000005\n");
      printf("  MB_ICONHAND        : 0x00000010\n");
      printf("  MB_ICONQUESTION    : 0x00000020\n");
      printf("  MB_ICONEXCLAMATION : 0x00000030\n");
      printf("  MB_ICONINFORMATION : 0x00000040\n");
      printf("  MB_DEFBUTTON1      : 0x00000000\n");
      printf("  MB_DEFBUTTON2      : 0x00000100\n");
      printf("  MB_DEFBUTTON3      : 0x00000200\n");
      printf("  MB_APPLMODAL       : 0x00000000\n");
      printf("  MB_SYSTEMMODAL     : 0x00001000\n");
      printf("  MB_TASKMODAL       : 0x00002000\n");
      printf("  MB_SETFOREGROUND   : 0x00010000\n");
      printf("\nsend bugs to <dmitry@karasik.eu.org>\n");
      exit(0);
   }

   strncpy( p1, argv[1], 1024);
   if ( argc > 2)
     strncpy( p2, argv[2], 1024);
   if ( argc > 3)
      flags = ( int ) strtoul( argv[3], NULL, 0);

   ret = MessageBox( NULL, p1, p2, flags);

   switch ( ret) {
     case IDABORT      : printf("ABORT"); break;
     case IDIGNORE     : printf("IGNORE"); break;
     case IDRETRY      : printf("RETRY"); break;
     case IDYES        : printf("YES"); break;
     case IDNO         : printf("NO"); break;
     case IDOK         : printf("OK"); break;
     default           : printf("CANCEL");
   }

   return ret;
}

