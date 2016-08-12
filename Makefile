all: minwins.exe

minwins.res: minwins.rc
	rc minwins.rc

minwins.obj: minwins.c
	cl -c minwins.c

list.obj: list.c list.h
	cl -c list.c

minwins.exe: minwins.obj list.obj minwins.res
	link /out:minwins.exe minwins.obj list.obj minwins.res user32.lib shell32.lib

clean:
	rm -f *.obj *.exe *.res
