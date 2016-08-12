#include <windows.h>
#include <stdio.h>

/* renames file in current directory so that they not contain unicode characters */

#ifndef WC_NO_BEST_FIT_CHARS
#  define WC_NO_BEST_FIT_CHARS 0x00000400 /* requires Windows 2000 or later */
#endif


int main( int argc, char * argv)
{
	HANDLE f;
	BOOL there_are_files = TRUE;
	WIN32_FIND_DATAW d;
	
	f = FindFirstFileW( L"*.*", &d);
	if ( f == INVALID_HANDLE_VALUE) {
	   	printf("FindFileFirst died\n");
		exit(1);
	}

	do {
	   	BOOL default_char_used;
		CHAR buffer[MAX_PATH+1];
		WideCharToMultiByte( 
			CP_OEMCP, WC_NO_BEST_FIT_CHARS,
			d. cFileName, -1,
			buffer, MAX_PATH, NULL, &default_char_used);
		if ( default_char_used) {
			CHAR *p = buffer;
			WCHAR buf2[MAX_PATH+1], *q = d. cFileName, *r = buf2;

			printf("%s\n", buffer);
			while ( *p) {
				if ( *p == '?') {
					*r = L'_';
				} else {
					*r = *q;
				}
				p++; q++; r++;
			}
			*r++ = 0;
			MoveFileW( d. cFileName, buf2);
		}
	   	there_are_files = FindNextFileW( f, &d);
	} while ( there_are_files);

	FindClose( f);

	return 0;
}
