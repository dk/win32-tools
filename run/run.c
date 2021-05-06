#include <stdio.h>
#include <string.h>
#include <process.h>

int main(int argc, const char ** argv)
{
	int i;
	char *narg[256], buf[2048], *ptr;
	if ( argc == 1 ) {
		printf("run PATH ARGS..\n");
		exit(1);
	}

	ptr = buf;
	for ( i = 1; i < argc; i++) {
		narg[i - 1] = ptr;
		*(ptr++) = '"';
		*ptr = 0;
		strcat( ptr, argv[i] );
		ptr += strlen( argv[i] );
		*(ptr++) = '"';
		*(ptr++) = 0;
	}
	narg[i - 1] = NULL;
	return _spawnvp( _P_NOWAIT, argv[1], (const char* const*) narg);
}
