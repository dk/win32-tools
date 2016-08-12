#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <shlobj.h>
#include <stdio.h>
#include <initguid.h>
#include <string.h>

HRESULT hres; 
IShellLink* psl = NULL; 
IPersistFile* ppf = NULL; 
WORD wsz[MAX_PATH]; 




HRESULT CreateLink(LPCSTR lpszPathObj, 
    LPSTR lpszPathLink, LPSTR lpszDesc) 
{ 
    IShellLink* psl; 
 
    // Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance(&CLSID_ShellLink, NULL, 
        CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl); 
    if (SUCCEEDED(hres)) { 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target, and add the 
        // description. 
        psl->lpVtbl->SetPath(psl, lpszPathObj); 
        psl->lpVtbl->SetDescription(psl, lpszDesc); 
 
       // Query IShellLink for the IPersistFile interface for saving the 
       // shortcut in persistent storage. 
        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, 
            &ppf); 
 
        if (SUCCEEDED(hres)) { 
            WORD wsz[MAX_PATH]; 
 
            // Ensure that the string is ANSI. 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, 
                wsz, MAX_PATH); 
 
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->lpVtbl->Save(ppf, wsz, TRUE); 
            ppf->lpVtbl->Release(ppf); 
        } else
           printf("2\n");
        
        psl->lpVtbl->Release(psl); 
    } else 
       printf("1\n");
    return hres; 
}

void 
die( char * s, ...)
{
   va_list va;
   va_start( va, s);
   if ( psl) psl->lpVtbl->Release(psl); 
   if ( ppf) ppf->lpVtbl->Release(ppf); 
   vfprintf( stderr, s, va);
   va_end( va);
   exit(1);
}   


int
main(int argc, char *argv[])
{
    int i, optstarted = 0, optended = 0;
    char * fn = NULL, * path = NULL, * desc = NULL, * icon = NULL, *args = NULL, * wkdir = NULL;
    int hasoptcreate = 0;
    int hasoptscmd = 0, firstarg = 0, iidx = 0, touch = 0;
    
#define XOPT(id); \
    if ( i == argc - 1) die("Option %s requires parameter\n", argv[i]);\
    id = argv[++i];\
    printf("** %s accepted as %s\n", id, #id);\
    touch = 1;
        

    for ( i = 1; i < argc; i++) {
       if (( argv[i][0] == '-') && strlen( argv[i]) == 2) {
          if ( optended) die("Too late for options\n");
          switch( argv[i][1]) {
          case 'c':
             hasoptcreate = 1; touch = 1;
             break;
          case 'p': XOPT(path); break;
          case 'd': XOPT(desc); break;
          case 'i': XOPT(icon); break;
          case 'a': XOPT(args); break;
          case 'w': XOPT(wkdir); break;
          case 's': {
             char * c, * ep = NULL;
             XOPT(c);
             hasoptscmd = strtoul( c, &ep, 0);
             if ( ep == c || hasoptscmd <= 0 || hasoptscmd > 3) 
                die("Invalid show window parameter:%s\n", c);
          } break;
          case 'j': {
             char * c, * ep = NULL;
             XOPT(c);
             iidx = strtoul( c, &ep, 0);
             if ( ep == c || iidx < 0) 
                die("Invalid icon index:%s\n", c);
          } break;
          }   
       } else {
          optended = 1;
          firstarg = i;
          fn = argv[i];
       }   
    }   

    if ( iidx > 0 && icon == NULL) die("No icon file specified while icon index is set\n");
    
    
    if (firstarg < 1) {
       printf("Syntax: lnk [options] filename.lnk\n"\
       "\n"\
       "options:\n"\
       " -c       create filename.lnk\n"\
       " -p path  sets path to a link\n"\
       " -d desc  sets description to a link\n"\
       " -i icon  sets icon to a link\n"\
       " -j idx   sets icon index to a link; needs -i to be set\n"\
       " -a args  sets command-line arguments to a program\n"\
       " -w path  sets working directory to a program\n"\
       " -s cmd   sets window show command, from 1 to 3. they are:\n"\
       "    1 - SW_SHOWNORMAL\n"\
       "    2 - SW_SHOWMINIMIZED\n"\
       "    3 - SW_SHOWMAXIMIZED\n"\
       "\n"\
       "send bugs to <dmitry@karasik.eu.org>\n"
       );
       return 0;
    }
    
    if ( !SUCCEEDED(CoInitialize(NULL)))
        die("Could not open the COM library\n");
    if ( !SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL, 
                                CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl)))
       die("Cannot create COM instance\n");
    if ( !SUCCEEDED( psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile,&ppf)))
       die("Cannot QueryInterface to IPersistFile\n");

    MultiByteToWideChar(CP_ACP, 0, fn, -1, wsz, MAX_PATH);    
    
    if ( !hasoptcreate) {
       if ( !SUCCEEDED( ppf->lpVtbl->Load(ppf, wsz, STGM_READ)))
          die("Cannot load %s\n", fn);
    }   
    
    if ( touch) {
      if ( path)
         psl->lpVtbl->SetPath(psl, path); 
      if ( wkdir)
         psl->lpVtbl->SetWorkingDirectory(psl, wkdir); 
      if ( desc)
         psl->lpVtbl->SetDescription(psl, desc); 
      if ( args)
         psl->lpVtbl->SetArguments(psl, args); 
      if ( icon)
         psl->lpVtbl->SetIconLocation(psl, icon, iidx); 
      if ( hasoptscmd)
         psl->lpVtbl->SetShowCmd(psl, hasoptscmd);      
      
      if ( !SUCCEEDED(ppf->lpVtbl->Save(ppf, wsz, TRUE)))
         die("Cannot write to %s\n", fn);
    } else {
       char wc[MAX_PATH];
       int icon;
       WIN32_FIND_DATA pfd;
       psl->lpVtbl->GetDescription( psl, wc, MAX_PATH);
       printf( wc); printf("\n");
       psl->lpVtbl->GetPath( psl, wc, MAX_PATH, &pfd, SLGP_UNCPRIORITY);
       printf("Path      :%s\n", wc);
       psl->lpVtbl->GetWorkingDirectory( psl, wc, MAX_PATH);
       printf("Work dir  :%s\n", wc);
       psl->lpVtbl->GetArguments( psl, wc, MAX_PATH);
       printf("Arguments :%s\n", wc);
       psl->lpVtbl->GetIconLocation( psl, wc, MAX_PATH, &icon);
       printf("Icon      :%s,%d\n", wc, icon);
       psl->lpVtbl->GetShowCmd( psl, &icon);
       printf("Show cmd  :");
       switch( icon) {
       case SW_SHOWNORMAL:printf("SW_SHOWNORMAL\n"); break;
       case SW_SHOWMINIMIZED:printf("SW_SHOWMINIMIZED\n"); break;
       case SW_SHOWMAXIMIZED:printf("SW_SHOWMAXIMIZED\n"); break;
       }   
    }   

    ppf->lpVtbl->Release(ppf); 
    psl->lpVtbl->Release(psl); 

    // CreateLink("c:\\home\\misc\\enumc\\lnk.exe", "c:\\a.lnk", "Oba!");
    
    return 0;
}   
          
