#include <windows.h>

int main( int argc, char**argv)
{
    unsigned int i, nd = waveOutGetNumDevs(), ret;
    DWORD a1, a2;
    
    printf("Number of wave devices: %d\n", nd);

    for ( i = 0; i < nd; i++) {
        WAVEOUTCAPS w;
	memset( &w, 0, sizeof(w));
        ret = waveOutGetDevCaps( i, &w, sizeof(w));
        printf("Device %d: %s\n", i, w.szPname);
    }

    a1 = -1;
    ret = waveOutMessage(( HWAVEOUT) WAVE_MAPPER, DRVM_MAPPER_PREFERRED_GET, a1, a2);
    printf("Preferred device: %d (%d)\n", a1, ret);

    return 0;
}
