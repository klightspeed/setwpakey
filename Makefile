CC=gcc
AS=as
DLLTOOL=dlltool
WINDRES=windres
WINE=

.PHONY: all test clean distclean

all: autowpakey.exe wpakeygui.exe

clean:
	rm -f *.o *.a *~ mkwlankeys.exe wlankeys.h

distclean:
	rm -f *.o *.a *~ .*.sw? *.exe wlankeys.h

wlankeys.h: mkwlankeys.exe
	$(WINE) ./$< >$@

autowpakey.exe: autowpakey.o setwpakey.o resource.o tea.o libwlanapi.a 
	$(CC) -o $@ $+ -mconsole

wpakeygui.exe: wpakeygui.o setwpakey.o dialog.o tea.o libwlanapi.a
	$(CC) -o $@ $+ -mwindows

mkwlankeys.exe: mkwlankeys.o tea.o
	$(CC) -o $@ $+ -lkernel32

autowpakey.o: autowpakey.c setwpakey.h wlankeys.h wlankey.h tea.h

wpakeygui.o: wpakeygui.c setwpakey.h wlankeys.h wlankey.h tea.h

setwpakey.o: setwpakey.c wlankeys.h wlankey.h tea.h

mkwlankeys.o: mkwlankeys.c wlankey.h tea.h wlankeys.ini

tea.o: tea.c tea.h

resource.o: resource.rc
	$(WINDRES) -o $@ $+

dialog.o: dialog.rc
	$(WINDRES) -o $@ $+

libwlanapi.a: wlanapi.def
	$(DLLTOOL) --as=$(AS) -k --output-lib $@ --def $<
