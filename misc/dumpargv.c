#include <stdio.h>
#include <fcntl.h>

int main(int argc, char ** argv)
{
	int i, f;
	f = fopen( "c:\\tmp\\dumpargv.log", "a");

	fprintf( f, "==============================\n");
	for ( i = 0; i < argc; i++) 
		fprintf( f, "%s\n", argv[i]);
	fclose(f);
	return 0;
}
