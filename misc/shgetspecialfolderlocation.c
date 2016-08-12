#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <shlobj.h>
#include <stdio.h>
#include <initguid.h>
#include <string.h>


int ids[] = {
CSIDL_APPDATA,
CSIDL_BITBUCKET,
CSIDL_COMMON_DESKTOPDIRECTORY,
CSIDL_COMMON_PROGRAMS ,
CSIDL_COMMON_STARTMENU ,
CSIDL_COMMON_STARTUP ,
CSIDL_CONTROLS ,
CSIDL_DESKTOP ,
CSIDL_DESKTOPDIRECTORY ,
CSIDL_DRIVES ,
CSIDL_FONTS ,
CSIDL_NETHOOD ,
CSIDL_NETWORK ,
CSIDL_PERSONAL ,
CSIDL_PRINTERS ,
CSIDL_PRINTHOOD,
CSIDL_PROGRAMS ,
CSIDL_RECENT ,
CSIDL_SENDTO ,
CSIDL_STARTMENU ,
CSIDL_STARTUP ,
CSIDL_TEMPLATES 
};   

char * strings[] = {
"CSIDL_APPDATA",
"CSIDL_BITBUCKET",
"CSIDL_COMMON_DESKTOPDIRECTORY",
"CSIDL_COMMON_PROGRAMS",
"CSIDL_COMMON_STARTMENU",
"CSIDL_COMMON_STARTUP",
"CSIDL_CONTROLS",
"CSIDL_DESKTOP",
"CSIDL_DESKTOPDIRECTORY",
"CSIDL_DRIVES",
"CSIDL_FONTS",
"CSIDL_NETHOOD",
"CSIDL_NETWORK",
"CSIDL_PERSONAL",
"CSIDL_PRINTERS",
"CSIDL_PRINTHOOD",
"CSIDL_PROGRAMS",
"CSIDL_RECENT",
"CSIDL_SENDTO",
"CSIDL_STARTMENU",
"CSIDL_STARTUP",
"CSIDL_TEMPLATES"
};   


int 
main( int argc, char * argv[]) 
{ 
    LPITEMIDLIST pidlPrograms; 
    LPMALLOC g_pMalloc;
    char wc[ MAX_PATH];
    char * ep;
    int id;

    if ( argc < 2) {
       int i;
       printf("format: SHGetSpecialFolderLocation FOLDER_ID\n"\
       "FOLDER_ID's:\n");
       for ( i = 0; i < sizeof(ids)/sizeof(int); i++) 
          printf("%s - %d\n", strings[i], ids[i]);
       printf("\nsend bugs to <dmitry@karasik.eu.org>\n");
       return 0;       
    }   
    id = strtol( argv[1], &ep, 0);
    if ( ep == argv[1] || id < 0) {
       printf("Invalid ID given\n");
       return 1;
    }   
    
 
    // Get the shell's allocator. 
    if (!SUCCEEDED(SHGetMalloc(&g_pMalloc))) 
        return 1; 
 
    // Get the PIDL for the Programs folder. 
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, 
            id, &pidlPrograms))) { 
        int i, j = -1;
        SHGetPathFromIDList( pidlPrograms, wc);
        for ( i = 0; i < sizeof(ids)/sizeof(int); i++)
           if ( ids[i] == id) {
              j = i;
              break;
           }   
        printf(( j >= 0 ) ? strings[j] : "");
        printf( "\n");
        printf( wc);
        printf( "\n");
 
        // Free the PIDL for the Programs folder. 
        g_pMalloc->lpVtbl->Free(g_pMalloc, pidlPrograms); 
    } 
 
    // Release the shell's allocator. 
    g_pMalloc->lpVtbl->Release(g_pMalloc); 
    return 0; 
} 

