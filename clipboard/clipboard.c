#include <windows.h>
#include <stdio.h>

static struct {
   UINT format;
   char desc[ 20];
} formats[] = {
      {CF_TEXT             , "CF_TEXT"
   }, {CF_BITMAP           , "CF_BITMAP"
   }, {CF_METAFILEPICT     , "CF_METAFILEPICT"
   }, {CF_SYLK             , "CF_SYLK"
   }, {CF_DIF              , "CF_DIF"
   }, {CF_TIFF             , "CF_TIFF"
   }, {CF_OEMTEXT          , "CF_OEMTEXT"
   }, {CF_DIB              , "CF_DIB"
   }, {CF_PALETTE          , "CF_PALETTE"
   }, {CF_PENDATA          , "CF_PENDATA"
   }, {CF_RIFF             , "CF_RIFF"
   }, {CF_WAVE             , "CF_WAVE"
   }, {CF_UNICODETEXT      , "CF_UNICODETEXT"
   }, {CF_ENHMETAFILE      , "CF_ENHMETAFILE"
   }, {CF_HDROP            , "CF_HDROP"    
   }, {CF_LOCALE           , "CF_LOCALE"          
   }, {CF_MAX              , "CF_MAX"         
   }
};

static char * 
err_msg(void)
{
   static char buf[ 256];
   LPVOID lpMsgBuf;
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      ( LPTSTR) &lpMsgBuf, 0, NULL);
   if ( lpMsgBuf)
      strncpy( buf, ( const char *) lpMsgBuf, 256);
   else
      buf[0] = 0;
   buf[ 255] = 0;
   LocalFree( lpMsgBuf);
   return buf;
}

static char * 
fmt2text( UINT f) 
{
   int i = 0;
   static char name[256];
   while ( formats[i]. format != CF_MAX) {
      if ( formats[i]. format == f) 
         return formats[i]. desc;
      i++;
   }
   if ( GetClipboardFormatName( f, name, 255))
      return name;
   return NULL;
}

void main( int argc, char ** argv) 
{
   char action, * parm = NULL;
   UINT f;
   
   if ( argc <= 1) {
      printf(
"format: clipboard [options]\n" \
"options:\n"\
"   -c         clear cliboard\n"\
"   -e         enumerate clipboard formats\n"\
"   -p FMT     print format FMT entry\n"\
"   -r FMT     register a clipboard format FMT and print out the atom value\n"\
"   -s FMT     read data from stdin and store as format FMT\n"\
"   -S FMT     same as -s, but do not empty the clipboard before\n"\
"   -l layout  select keyboard layout string ( see LoadKeyboardLayout() win32 refs. )\n"\
"   \n"\
"note: The FMT parameter can be specified either as a text string or an\n"\
"integer value, except with the '-r' key, which always accepts FMT as a\n"\
"text string\n"\
"\nsend bugs to <dmitry@karasik.eu.org>\n"
            );
      exit(0);
   } 

   
   {
      int i, got_option = 0;
      for ( i = 1; i < argc; i++) {
         if ( strlen( argv[i]) != 2 || argv[i][0] != '-') {
            fprintf( stderr, "Invalid parameter: '%s'\n", argv[i]);
            exit(1);
         }
         if ( strchr( "lprsS", argv[i][1]) != NULL) {
            if ( i == argc - 1 ) {
               fprintf( stderr, "Too few parameters\n");
               exit(1);
            } 
            if ( argv[i][1] == 'l') {
               if ( !( LoadKeyboardLayout( argv[i + 1], KLF_ACTIVATE|KLF_REORDER )))
                  fprintf( stderr, "LoadKeyboardLayout() failed: %s\n", err_msg());
            } else {
               parm = argv[i + 1];
               if ( got_option) {
                  fprintf( stderr, "'%s' option is ambiguous\n", argv[i]);
                  exit(1);
               }
               got_option = 1;
               action = argv[i][1];
            }
            i++;
         } else if ( strchr( "ce", argv[i][1]) != NULL) {
            if ( got_option) {
               fprintf( stderr, "'%s' option is ambiguous\n", argv[i]);
               exit(1);
            }
            got_option = 1;
            action = argv[i][1];
         } else {
            fprintf( stderr, "Unknown options: '%s'\n", argv[i]);
            exit(1);
         }
      }
   }

   if ( !OpenClipboard( NULL)) {
      fprintf( stderr, "OpenClipboard() failed: %s\n", err_msg());
      exit(1);
   }

   
   switch ( action) {
   case 'c':
      if ( !EmptyClipboard()) 
         fprintf( stderr, "EmptyClipboard() failed: %s\n", err_msg());
      break;
   case 'r':
      f = RegisterClipboardFormat( parm );
      if ( f == 0)
         fprintf( stderr, "RegisterClipboardFormat() failed: %s\n", err_msg());
      printf("%d\n", f);
      break;
   case 'e':
      f = 0;
      while (( f = EnumClipboardFormats( f)))
         printf("%8d  %s\n", f, fmt2text( f));
      break;
   default:
      {
         char * e;
         f = strtol( parm, &e, 0);
         if ( *e != '\0') {
            UINT xf = 0;
            f = 0;
            while (( xf = EnumClipboardFormats( xf))) {
               if ( strcmp( parm, fmt2text( xf)) == 0) {
                  f = xf;
                  break;
               }
            }
            if ( f == 0) {
               f = RegisterClipboardFormat( parm);
               if ( f == 0) {
                  fprintf( stderr, "RegisterClipboardFormat() failed: %s\n", err_msg());
                  goto FINALIZE;
               }
            } 
         } 
      }
      switch ( action) {
      case 'p': 
         {
            void * x;
            HGLOBAL g;

            if ( !IsClipboardFormatAvailable( f))
               goto FINALIZE;
            
            g = GetClipboardData( f);
            if ( !g) {
               fprintf( stderr, "GetClipboardData() failed:%s\n", err_msg());
               break;
            }
            if ( !( x = GlobalLock( g))) {
               fprintf( stderr, "GlobalLock() failed:%s\n", err_msg());
               break;
            }
            fwrite( x, GlobalSize( g), 1, stdout);
            GlobalUnlock( g);
         }
         break;
      case 's':
      case 'S':
         {
            int vmsz = 1024, sz = 0, r;
            char * vm = malloc( vmsz);
            void * x;
            HGLOBAL g;
            while ( 1) {
               if ( vmsz == sz) {
                  char * nvm = malloc( vmsz * 2);
                  if ( !nvm) {
                     free( vm);
                     fprintf( stderr, "Not enough memory: %d bytes\n", vmsz * 2);
                     goto FINALIZE;
                  }
                  memcpy( nvm, vm, vmsz);
                  free( vm);
                  vm = nvm;
                  vmsz *= 2;
               }
               r = fread( vm + sz, 1, vmsz - sz, stdin);
               /* printf("Read %d out of %d bytes [%02x %02x] @ %d\n", r, vmsz - sz, vm[sz], vm[sz+1], sz); */
               if ( r != vmsz - sz) {
                  if ( r > 0) sz += r;
                  break;
               } 
               sz += r;
            }
            g = GlobalAlloc( GMEM_DDESHARE, sz); 
            if ( !g) {
               fprintf( stderr, "GlobalAlloc() failed with %d bytes:%s\n", sz, err_msg());
               goto FINALIZE;
            }
            if ( !( x = GlobalLock( g))) {
               GlobalFree( g);
               fprintf( stderr, "GlobalLock() failed:%s\n", err_msg());
            }
            memcpy( x, vm, sz);
            free( vm);
            GlobalUnlock( g);

            if ( action == 's') 
               if ( !EmptyClipboard())
                 fprintf( stderr, "EmptyClipboard() failed: %s\n", err_msg());
            
            if ( !SetClipboardData( f, g)) {
               fprintf( stderr, "SetClipboardData() failed: %s\n", err_msg());
               goto FINALIZE;
            }
         }
         break;
      }
   }

FINALIZE:;   
   if ( !CloseClipboard()) {
      fprintf( stderr, "CloseClipboard() failed: %s\n", err_msg());
      exit(1);
   }
}
   
