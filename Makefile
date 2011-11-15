CC=gcc
STRIP=strip
AS=as
DLLTOOL=dlltool
WINDRES=windres
WINE=

.PHONY: all test clean distclean

all: setwpakey.exe wpakeygui.exe

clean:
	rm -f *.o *.a *~ mkwlankeys.exe wlankeys.h

distclean:
	rm -f *.o *.a *~ .*.sw? *.exe wlankeys.h

wlankeys.h: mkwlankeys.exe
	$(WINE) ./$< >$@

setwpakey.exe: setwpakey.o wlankey.o resource.o tea.o libwlanapi.a 
	$(CC) -o $@ $+ -mconsole -lntdll
	$(STRIP) --strip-unneeded $@

wpakeygui.exe: wpakeygui.o wlankey.o dialog.o tea.o libwlanapi.a
	$(CC) -o $@ $+ -mwindows -lntdll
	$(STRIP) --strip-unneeded $@

mkwlankeys.exe: mkwlankeys.o tea.o
	$(CC) -o $@ $+ -lkernel32 -lntdll
	$(STRIP) --strip-unneeded $@

setwpakey.o: setwpakey.c wlankey.h tea.h

wpakeygui.o: wpakeygui.c wlankey.h tea.h

wlankey.o: wlankeys.h wlankey.h tea.h

mkwlankeys.o: mkwlankeys.c wlankey.h tea.h wlankeys.ini

tea.o: tea.c tea.h

resource.o: resource.rc
	$(WINDRES) -o $@ $+

dialog.o: dialog.rc
	$(WINDRES) -o $@ $+

libwlanapi.a: wlanapi.def
	$(DLLTOOL) --as=$(AS) -k --output-lib $@ --def $<
