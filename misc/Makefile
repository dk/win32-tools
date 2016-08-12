all: ddeclient.exe enablewindow.exe enumwindows.exe lnk.exe messagebox.exe setwindowpos.exe shgetspecialfolderlocation.exe 

ddeclient.exe: ddeclient.c
	cl ddeclient.c user32.lib shell32.lib ole32.lib

enablewindow.exe: enablewindow.c
	cl enablewindow.c user32.lib shell32.lib ole32.lib

enumwindows.exe: enumwindows.c
	cl enumwindows.c user32.lib shell32.lib ole32.lib

lnk.exe: lnk.c
	cl lnk.c user32.lib shell32.lib ole32.lib

messagebox.exe: messagebox.c
	cl messagebox.c user32.lib shell32.lib ole32.lib

setwindowpos.exe: setwindowpos.c
	cl setwindowpos.c user32.lib shell32.lib ole32.lib

shgetspecialfolderlocation.exe: shgetspecialfolderlocation.c
	cl shgetspecialfolderlocation.c user32.lib shell32.lib ole32.lib

clean:
	rm -f *.exe *.obj