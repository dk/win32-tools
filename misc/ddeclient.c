#include <windows.h>
#include <stdio.h>

void 
die( char * s, ...)
{
   va_list va;
   va_start( va, s);
   vfprintf( stderr, s, va);
   va_end( va);
   exit(1);
}  

char xmdbuf[ 1024];

char * err_msg( DWORD errId)
{
   LPVOID lpMsgBuf;
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errId,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      ( LPTSTR) &lpMsgBuf, 0, NULL);
   if ( lpMsgBuf) {
     strncpy( xmdbuf, lpMsgBuf, 1024);
     LocalFree( lpMsgBuf);
   } else
     sprintf( xmdbuf, "%04x", errId);
   xmdbuf[ 1024] = 0;
   return xmdbuf;
}

int main( int argc, char ** argv)
{
   DWORD inst = 0;
   HSZ   ps;
   HCONV conv;
   DWORD res;   
   int lvl = 0;

   if ( argc < 3) {
      printf( "DDE execution client. Performs DdeClientTransaction with XTYP_EXECUTE flag.\n"\
      "\n"\
      "format: ddeclient instance service\n"\
      "examples:\n"\
      "  PROGMAN [CreateGroup(WinHack)]\n"\
      "  PROGMAN [AddItem(c:/winhack.exe,WinHack)]\n"\
      "\n"\
      "For more info refer to MSDK, UI Services/Shell/Shell DDE Interface.\n"\
      "send bugs to <dmitry@karasik.eu.org>\n"
      );
      exit(0);
   }   
   
   res = DdeInitialize( &inst, NULL, APPCMD_CLIENTONLY|CBF_SKIP_ALLNOTIFICATIONS, 0);
   if ( res != DMLERR_NO_ERROR)
      die( "DdeInitalize error:%s", err_msg(res));

   ps   = DdeCreateStringHandle( inst, argv[1], CP_WINANSI);
   conv = DdeConnect( inst, ps, ps, NULL);
   if ( !conv) {
      DdeFreeStringHandle ( inst, ps);
      DdeUninitialize ( inst);
      die( "DdeConnect error:%s", err_msg(res));
   }

AGAIN:
   if ( !DdeClientTransaction( argv[2], strlen( argv[2]), conv, 0, 0, XTYP_EXECUTE, 10000, &res)) {
      res = DdeGetLastError( inst);
      if ( res == 0x4009 && lvl++ < 5) {
         // refuse
         Sleep( 300);
         goto AGAIN;
      }
      DdeFreeStringHandle ( inst, ps);
      DdeDisconnect( conv);
      DdeUninitialize ( inst);
      die( "DdeClientTransaction error:%s\n", err_msg( res));
   }

   DdeFreeStringHandle ( inst, ps);
   DdeDisconnect( conv);
   DdeUninitialize ( inst);

   exit(0);
   return 1;   
}   
