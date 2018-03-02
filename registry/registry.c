#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

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
   char * c;
   LPVOID lpMsgBuf;
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, errId,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      ( LPTSTR) &lpMsgBuf, 0, NULL);
   if ( lpMsgBuf) {
     strncpy( xmdbuf, lpMsgBuf, 1024);
     LocalFree( lpMsgBuf);
   } else
     sprintf( xmdbuf, "%04x", errId);
   c = xmdbuf;
   while( *(c++)) if (( *c) == '\n' || ( *c) == '\r') *c = ' ';
   xmdbuf[ 1024] = 0;
   return xmdbuf;
}

#define TYP_0 0
#define TYP_S 1
#define TYP_I 2
#define TYP_R 3
#define TYP_X 4

#define KEY ( key ? key : "(.Default)")

static char * types[] = {
   "REG_NONE",
   "REG_SZ",
   "REG_EXPAND_SZ",
   "REG_BINARY",
   "REG_DWORD",
   "REG_DWORD_BIG_ENDIAN",
   "REG_LINK",
   "REG_MULTI_SZ",
   "REG_RESOURCE_LIST",
};

static int hasoptcreate = 0, hasoptdelete = 0, hasenum = 0, hasoptdebug = 0, printmode = 0, hasoptprint = 0;
static int haskey = 0, hasvalue = 0, hasoptvalfile = 0, valsize = 0, inpf = TYP_0, outf = TYP_0, typeset = 0;
static int valmalloced = 0, hasopttype = 0;

void
export_key( char * key, DWORD type, int vlen, char * valbuf)
{
   int needcolon = 0, otyp = outf, i;
   if ( key) {
      printf( key[0] == '\0' ? "(.Default)" : key);
      needcolon = 1;
   }   
   
   if ( printmode == 1 || printmode == 3) {
      if ( needcolon)
         printf( " : ");
      printf(( type < 0 || type > REG_RESOURCE_LIST) ? "unknown" : types[type]);
      needcolon = 1;
   }   
   if ( printmode == 2 || printmode == 3) {
      if ( needcolon)
         printf( " : ");      
      needcolon = 1;
      if ( otyp == TYP_0) switch ( type) {
      case REG_SZ:
      case REG_EXPAND_SZ:
         otyp = TYP_S;
         break;
      case REG_DWORD:
         otyp = TYP_I;
         break;
      default:
         otyp = TYP_X;   
      }
      
      switch ( otyp) {
      case TYP_S:
         if ( valbuf[vlen-1] == 0) vlen--;
         fwrite( valbuf, 1, vlen, stdout);
         break;
      case TYP_I:
         printf( "%ld", ( long) *((DWORD*)valbuf));
         break;
      case TYP_X:
         for ( i = 0; i < vlen; i++)
            printf( "%02x", valbuf[ i]);
         break; 
      default:
         fwrite( valbuf, 1, vlen, stdout);
      }   
   }  
   printf("\n");
}   

void main( int argc, char ** argv)
{
   int i;
   HKEY hkey = 0;
   REGSAM access = KEY_ALL_ACCESS;
   char * path, * key = NULL, * value = NULL;
   if ( argc < 2) {
      printf("format: registry [options] HKEY path [ key [value]]\n"\
             "options:\n"
             " -c    create key\n"\
             " -d    delete key\n"\
             " -e    enumerate subpaths ( vs. keys, by default)\n"\
             " -f    treat value name as file, to read from. Empty string - stdin\n"\
             " -iX   input data format, see -hi\n"\
             " -kX   key type format, see -hf\n"\
             " -oX   output data format, see -hi\n"\
             " -h[X] print help [ for specific topic]\n"\
             " -pX   key print format, see -hp\n"\
             " -x    print excessive debug information\n"\
             " \n"\
             "note: to specify a root path, or a (.default) key name, use empty string\n"\
             " \n"\
             "send bugs to <dmitry@karasik.eu.org>\n"
      );
      exit(0);
   }

   

   // options
   {
      int hasopt = 0;
      int noopt  = 0;
      for ( i = 1; i < argc; i++) {
         if ( hasoptdebug) fprintf( stderr, "** ARGV[%d]:%s\n", i, argv[i]);
         if ( argv[i][0] == '-') {
            int j, len = strlen( argv[i]);
            hasopt = 1;

            if ( noopt) 
               die("Too late for option %s. Specify it earlier\n", argv[i]);
            
            for ( j = 1; j < len; j++) {
               char opt = argv[i][j];
               if ( hasoptdebug) fprintf( stderr, "** option :%c\n", opt);
               if ( opt == 'h') {
                  switch( argv[i][j+1]) {
                  case 'h':
                     printf( "abbreviations to HKEYs are: \n"\
                        "HKCR - HKEY_CLASSES_ROOT\n"\
                        "HKCU - HKEY_CURRENT_USER\n"\
                        "HKU  - HKEY_USERS\n"\
                        "HKCC - HKEY_CURRENT_CONFIG\n"\
                        "HKLM - HKEY_LOCAL_MACHINE\n"\
                        "HKDD - HKEY_DYN_DATA\n"
                     );
                     break;   
                  case 'p':
                     printf("key and value print format: -pX, where\n"\
                        "X could be:\n"\
                        "0 - print only key names\n"\
                        "1 or t - print types\n"\
                        "2 or v - print values\n"\
                        "3 or b - print types and values\n"\
                        "\n"\
                        "0 is default when no value is given,\n"\
                        "2 is default when value is specified.\n"\
                        "\n"\
                        "note: when value given, key name not printed, unless explicitly -p0 is set\n"
                     );  
                     break;     
                  case 'f':
                     printf( "data format codes, used to specify how to treat passed data.\n"\
                     "useful when input data are binary ( see -hi,x), and registry key is REG_MULTI_SZ, for example.\n"\
                      "0 - corresponding to existing format ( cannot be used with -c)\n"\
                      "1 or s - REG_SZ\n"\
                      "2 or e - REG_EXPAND_SZ\n"\
                      "3 or b - REG_BINARY\n"\
                      "4 or d - REG_DWORD\n"\
                      "5 or q - REG_DWORD_BIG_ENDIAN\n"\
                      "6 or l - REG_LINK\n"\
                      "7 or m - REG_MULTI_SZ\n"\
                      "8 or r - REG_RESOURCE_LIST\n"
                     ); 
                     break;       
                  case 'i':
                     printf("input/output format codes\n\n"\
                       "1 or s - string format\n"\
                       "2 or i or d - integer format\n"\
                       "3 or r or b - raw binary format\n"\
                       "4 or x - hexadecimal binary format\n"\
                       "0 - type-dependent; \n"\
                       "  REG_SZ and REG_EXPAND_SZ match for 's'\n"\
                       "  RED_DWORD - for 'i', and\n"\
                       "  other types - for 'x'.\n"
                     );       
                     break;
                  default:
                     printf( "-hX - prints help for topic X, where X could be:\n\n"\
                      "i - input/output formats\n"\
                      "p - key and value print formats\n"\
                      "f - registry type codes\n"\
                      "h - registry HKEY codes\n"\
                      "\n"
                     );
                  }
                  exit(0);
               } else if ( opt == 'c') { hasoptcreate = 1; continue;
               } else if ( opt == 'd') { hasoptdelete = 1; continue;
               } else if ( opt == 'e') { hasenum = 1; continue;
               } else if ( opt == 'x') { hasoptdebug = 1; continue;
               } else if ( opt == 'f') { hasoptvalfile = 1; continue;
               } else if ( opt == 'p') {
                  j++;
                  hasoptprint = 1;
                  switch ( argv[i][j]) {
                  case '0': case '1': case '2': case '3':
                     printmode = argv[i][j] - '0';
                     break;
                  case 't': printmode = 1; break;   
                  case 'v': printmode = 2; break;   
                  case 'b': printmode = 3; break;   
                     break;
                  default:   
                     die( "Invalid -p sub-option, see -hp for details\n");
                  }   
               } else if ( opt == 'i' || opt == 'o') {
                  int * mod = ( opt == 'i') ? &inpf : &outf;
                  j++;
                  switch( argv[i][j]) {
                  case '0': case '1': case '2': case '3': case '4': 
                     *mod = argv[i][j] - '0'; break;
                  case 's': *mod = TYP_S; break;
                  case 'i': case 'd' : *mod = TYP_I; break;
                  case 'r': case 'b' : *mod = TYP_R; break;
                  case 'x': *mod = TYP_X; break;
                  default:
                     die( "Invalid data format suboption, see -hi for details\n");
                  }   
               } else if ( opt == 'k') {
                  j++;
                  if ( opt == 'k') hasopttype   = 1;
                  switch( argv[i][j]) {
                  case '0': 
                  case '1': case '2':
                  case '3': case '4': case '5':
                  case '6': case '7': case '8':
                     typeset = argv[i][j] - '0';
                     break;
                  case 's' : typeset = REG_SZ; break;   
                  case 'e' : typeset = REG_EXPAND_SZ; break;   
                  case 'd' : typeset = REG_DWORD; break;   
                  case 'q' : typeset = REG_DWORD_BIG_ENDIAN; break;   
                  case 'b' : typeset = REG_BINARY; break;   
                  case 'l' : typeset = REG_LINK; break;   
                  case 'm' : typeset = REG_MULTI_SZ; break;   
                  case 'r' : typeset = REG_RESOURCE_LIST; break;   
                  default:
                     die( "Invalid type format suboption, see -hf for details\n");
                  }   
               } else 
                  die( "Unknown option:%c\n, see -h for details", opt);
            } 
         } else {
            noopt = 1;
            if ( hasopt) break;
         }   
      }

      if ( !hasopt) i = 1;


      if ( hasoptdelete && (
           hasoptcreate || hasoptvalfile || hasopttype || hasoptprint                 
        ))
         die("Cannot accept additional options on delete action\n");
      
      if ( hasoptdebug) fprintf( stderr, "** first actual parm:%d\n", i);
   }   

   // hkey
   if ( argv[ i]) {
      if ( stricmp( argv[ i], "HKLM") == 0) hkey = HKEY_LOCAL_MACHINE; else
      if ( stricmp( argv[ i], "HKCR") == 0) hkey = HKEY_CLASSES_ROOT; else
      if ( stricmp( argv[ i], "HKCU") == 0) hkey = HKEY_CURRENT_USER; else
      if ( stricmp( argv[ i], "HKU") == 0)  hkey = HKEY_USERS; else
      if ( stricmp( argv[ i], "HKCC") == 0) hkey = HKEY_CURRENT_CONFIG; else
      if ( stricmp( argv[ i], "HKDD") == 0) hkey = HKEY_DYN_DATA; else
         die("Unknown HK : %s\n. Should be one of HKLM, HKCR, HKCU, HKU, HKCC or HKDD\n", argv[i]);
      if ( hasoptdebug) fprintf( stderr, "** parm %d as HK: %s (0x%08x)\n", i, argv[i], (unsigned int)hkey);
   } else 
      die( "No HK code given\n"); 

   // path
   i++;
   if ( argv[i]) {
      if ( hasoptdebug) fprintf( stderr, "** parm %d as path: %s\n", i, argv[i]);
      path = argv[i];
      if ( path[0] == '\0') {
         path = NULL;
         if ( hasoptdebug) fprintf( stderr, "** root path accepted\n");
      }   
   } else 
      die( "No path given. If wanted to use root tree, use an empty string as parameter\n");
   
   // key
   i++;
   if ( argv[i]) {
      haskey = 1;
      if ( hasoptdebug) fprintf( stderr, "** parm %d as key: %s\n", i, argv[i]);
      key = argv[i];
      if ( key[0] == '\0') {
         key = NULL;
         if ( hasoptdebug) fprintf( stderr, "** default key accepted\n");
      }   
      if ( !hasoptprint) printmode = 1;
      i++;
      // value
      if ( argv[i]) {
         if ( hasoptdebug) fprintf( stderr, "** parm %d as %s%s\n", i, 
            hasoptvalfile ? ( strlen( argv[i]) ?  "file: " : "stdin") : "value: ", argv[i]);
         value = argv[i];
         hasvalue = 1;
         if ( hasoptvalfile) {
            int rd = 0, r, f = 0 /* stdin */;
            int filed = strlen( value);
            char * val; 
            valsize = 256;
            
            valmalloced = 1;
            if ( filed) {
               f = open( value, O_BINARY);
               if ( f == -1)
                  die("Cannot open file: %s\n", value);
            } 
            
            if ( !( val = malloc( 256))) {
               if ( filed) close(f);
               die( "Error allocationg 256 bytes\n");
            }   
            
            if ( hasoptdebug) fprintf( stderr, "** reading ...");
            while(( r = read( f, val + rd, 256)) > 0) {
               if ( !realloc( val, rd + r)) {
                  if ( filed) close(f);
                  free( val);
                  die( "Error allocating %d bytes\n", rd + r);
               }   
               rd += r;
            }   
            if ( filed) close(f);
            if ( hasoptdebug) fprintf( stderr, "** done, %d bytes read\n", rd);
            value = val;
            valsize = rd;
         } else 
            valsize = strlen( value);

         // converting from input mode
         if ( inpf == TYP_0) {
            switch ( typeset) {
            case REG_SZ: case REG_EXPAND_SZ:
               inpf = TYP_S;
               break;
            case REG_DWORD: case REG_DWORD_BIG_ENDIAN:
               inpf = TYP_I;
               break;
            case REG_BINARY: case REG_LINK: case REG_MULTI_SZ: case REG_RESOURCE_LIST: 
               inpf = TYP_R;
               break;   
            case 0:
               inpf = TYP_S;
               break;   
            }  
            if ( hasoptdebug) 
               fprintf( stderr, typeset ?
                        "** input format treated as '%d' from key type\n" :
                        "** Cannot determine input format, treating as string\n"
                , inpf);
         }   
         
         switch ( inpf) {
         case TYP_I: {
            char * ep;            
            DWORD rc = ( DWORD) strtoul( value, &ep, 0);
            if ( ep == value) die("Not a numeric value specified\n");
            if ( valmalloced) free( value);
            value = malloc( valsize = sizeof( DWORD));
            memcpy( value, &rc, valsize);
            if ( typeset == 0) typeset = REG_DWORD;
            }
            break;
         case TYP_S:
            if ( typeset == 0) typeset = REG_SZ;
            break;   
         case TYP_R:
            if ( typeset == 0) typeset = REG_BINARY;
            break;
         case TYP_X:
            if ( typeset == 0) typeset = REG_BINARY;
            {
               int i, idx = 0, j = 0;
               int n[2];
               unsigned char * nv = malloc( valsize / 2);
               if ( !nv) die("Cannot allocate %d bytes\n", valsize / 2);
               
               for ( i = 0; i < valsize; i++) {
                  register char z = value[i];
                  if ( z >= '0' && z <= '9')
                     n[idx] = z - '0';
                  else if ( z >= 'A' && z <= 'F') 
                     n[idx] = z - 'A' + 0xA;
                  else if ( z >= 'a' && z <= 'f') 
                     n[idx] = z - 'a' + 0xA;
                  else if ( z == '\r' || z == '\n' || z == ' ' || z == '\t' || z == ',')
                     continue;
                  else
                     die("Invalid hexadecimal sign '%c'(%02x) at position %d\n", z, z, i);
                  
                  if ( idx) {
                     nv[ j++] = ( n[0] << 4) | n[1];
                     // printf("%d:[%d %d]-> %d{%d}\n", i, n[0], n[1], j, nv[j-1]);
                     idx = 0;
                  } else
                     idx = 1;   
               }   
               valsize = j;
               if ( valmalloced) free( value);
               value = nv;
               valmalloced = 1;
            }   
            break;
         }   
      } else { // no value
         if ( hasoptvalfile)
            die("Confusing -f option when no value given. To use stdin specify the empty string\n");
      }   
   } else { // no key
      access = KEY_READ;
      if ( hasoptdebug) fprintf( stderr, "** access mode set to KEY_READ\n");
      if ( hasoptvalfile)
           die("Confusing -f option when no value and no key given\n");
   }   

   // validations
   if ( hasoptdebug) fprintf( stderr, "** c:%d d:%d e:%d p:%d(exp:%d) k:%d i:%d o:%d\n", 
      hasoptcreate, hasoptdelete, hasenum, printmode, hasoptprint, typeset, inpf, outf
                            );
   if ( hasvalue) { 
      if ( hasoptdebug && hasoptcreate) fprintf( stderr, "** creating and setting value\n");
      if ( hasoptdelete) 
         die("Cannot delete a value - can only a key or a path\n");
   } else {
      if ( hasoptcreate && !hasopttype) typeset = 0;
   }   

   if ( haskey) {
      if ( hasenum)
         die( "Confusing sub-path enumeration when a key is specified\n");
      if ( !hasoptprint)
         printmode = 2;
   } else {
      if ( hasopttype && !hasoptcreate) 
         die( "Confusing key type specification without a key\n");
   }   

   if ( hasoptdelete) {
      LONG res;
      if ( haskey) {
         HKEY k;
         if ( hasoptdebug) fprintf( stderr, "** deleting key %s in %s\n", KEY, path);
         if (( res = RegOpenKeyEx( hkey, path, 0, KEY_ALL_ACCESS, &k)) != ERROR_SUCCESS) 
            die( "Error opening %s:%d(%s)\n", path, res, err_msg(res));
         if (( res = RegDeleteValue( k, key)) != ERROR_SUCCESS) {
            RegCloseKey( k);
            die("Error deleting %s %s:%d(%s)\n", path, KEY, res, err_msg(res));
         }   
         RegCloseKey( k);
      } else {   
         if ( hasoptdebug) fprintf( stderr, "** deleting path %s\n", path);
         if (( res = RegDeleteKey( hkey, path)) != ERROR_SUCCESS)
            die( "Cannot delete %s:%d(%s)\n", path, res, err_msg(res));
      } 
      exit(0); 
   } 
   
   if ( hasoptcreate) {
      LONG res;
      HKEY k;
      DWORD disp;
      int justcreated = 0;

      if ( hasoptdebug) fprintf( stderr, "** creating key %s\n", path);
      if (( res = RegCreateKeyEx( hkey, path, 0, "", 
         REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &k, &disp)) != ERROR_SUCCESS)
         die( "Error creating %s:%d(%s)\n", path, res, err_msg(res));

      if ( !haskey) {
         if ( hasoptdebug) fprintf( stderr, "** no key specified, exiting\n");
         RegCloseKey( k);
         exit(0);
      }   

      // creating bogus default value
      if ( !hasvalue) {
         if ( hasoptdebug) fprintf( stderr, "** creating default value\n");
         if ( typeset == 0) {
            typeset = REG_SZ;
            if ( disp == REG_OPENED_EXISTING_KEY) {
               DWORD type;
               if ( hasoptdebug) fprintf( stderr, "** path %s already exist\n", path);
               if ( RegQueryValueEx( k, key, NULL, &type, NULL, NULL) == ERROR_SUCCESS) 
                  typeset = type;
               else 
                  justcreated = 1;
            } else 
               justcreated = 1;
         } 
         if ( hasoptdebug) fprintf( stderr, "** typeset %s\n", types[ typeset]);
         if ( typeset == REG_DWORD || typeset == REG_DWORD_BIG_ENDIAN) {
            valmalloced = 1;
            value = malloc( valsize = sizeof( DWORD));
            memset( value, 0, valsize);
         } else {
            value = "";
            valsize = 1;
         }   
      } else if ( disp == REG_CREATED_NEW_KEY) 
         justcreated = 1;
      RegCloseKey( k);
      if ( justcreated) exit(0);
   }   // end create
   
   // opening   
   
   {
      HKEY k;
      LONG res;
         
REREAD:      
      res = RegOpenKeyEx( hkey, path, 0, access, &k);
      if ( res != ERROR_SUCCESS) 
         die( "Error opening %s:%d(%s)\n", path, res, err_msg(res));
      if ( hasoptdebug) fprintf( stderr, "** key %08x opened with access %08x\n", (unsigned int)key, (unsigned int)access);

      if ( haskey) {
         DWORD i = 0, ilen = 1023, type, vlen = 1024, res;
         char buf[1024];
         
         // seems that seq read and write ops without resetting troubles windows registry...
         if ( !hasvalue) {
            if (( res = RegQueryValueEx( k, key, NULL, &type, buf, &vlen)) != ERROR_SUCCESS) {
               if ( key) {
                  RegCloseKey(k);
                  if ( !hasoptcreate)
                     die( "Key %s doesn't exists:%d(%s)\n", key, res, err_msg(res));
               } else {
                  // empty default value, emulating
                  type = REG_SZ;
                  vlen = 0;
                  buf[0] = '\0';
               }   
	    }
         }
         
         if ( hasvalue) {
            LONG res;
            if ( typeset == 0)
               typeset = type;
            if ( hasoptdebug) fprintf( stderr, "** setting data to %08x as %s\n", (unsigned int)key, types[typeset]);
            // if ( hasoptdebug) fprintf( stderr, "** key:%s,typeset:%d,valsize:%d,value:%s", key, typeset, valsize, value);
            if (( res = RegSetValueEx( k, key, 0, typeset, value, valsize)) != ERROR_SUCCESS)
               fprintf( stderr, "Cannot write %d bytes to %s:%d(%s)\n", valsize, KEY, res, err_msg(res));
            else if ( hasoptdebug) {
               hasvalue = 0;
               RegCloseKey( k);
               goto REREAD;
            }   
         } else 
            // exporting value
            export_key( printmode ? NULL : KEY // explicit p0 will print key name
                       , type, vlen, buf);
      } else {
      // enumerating keys
         DWORD i = 0, ilen = 1023;
         char buf[1024];
         if ( hasenum) {
            while ( RegEnumKeyEx( k, i, buf, &ilen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
               printf( buf);
               printf( "\n");
               i++;
               ilen = 1023;
            }   
         } else {
            DWORD type, nv = 0, vlen = 0;
            char * valbuf;
            {
               DWORD sk, msk, mc, mvn, sd;
               FILETIME ft;
               RegQueryInfoKey( k, buf, &ilen, NULL, &sk, &msk, &mc, &nv, &mvn, &vlen, &sd, &ft);
               if ( hasoptdebug) fprintf( stderr, "** RegQueryInfoKey: "\
                 "class:%s,subkeys:%d,maxlen:%d,clslen:%d,values:%d,maxname:%d,maxdata:%d\n",
                 buf, sk, msk, mc, nv, mvn, vlen
               );                           
               ilen = 1023;
            }
            
            if (!( valbuf = malloc( vlen + 1))) {
               RegCloseKey( k);
               die( "Cannot allocate %d bytes\n", vlen + 1);
            }   
            
            for ( i = 0; i < nv; i++) {
               LONG res;
               res = RegEnumValue( k, i, buf, &ilen, NULL, &type, valbuf, &vlen);
               if ( res != ERROR_SUCCESS) {
                  fprintf( stderr, "Error reading key #%d:%d(%s)\n", i, res, err_msg(res));
                  continue;
               }   
               export_key( buf, type, vlen, valbuf);
               vlen = ilen = 1023;
            }   
            free( valbuf);
         }   
      }   
      RegCloseKey( k);
   }
   if ( valmalloced) free( value);
}
