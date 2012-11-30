CC=gcc
STRIP=strip
AS=as
DLLTOOL=dlltool
WINDRES=windres
WINE=

.PHONY: all test clean

all: setwpakey.exe wpakeygui.exe autowpakey.exe getwpaconf.exe

clean:
	rm -f *.o *.a *~ .*.sw? *.exe wlankeys.h

wlankeys.h: mkwlankeys.exe wlankeys.ini
	$(WINE) ./$< >$@

setwpakey.exe: setwpakey.o wlankey.o resource.o tea.o libwlanapi.a 
	$(CC) -o $@ $+ -mconsole -lntdll
	$(STRIP) --strip-unneeded $@

wpakeygui.exe: wpakeygui.o wlankey.o dialog.o tea.o libwlanapi.a
	$(CC) -o $@ $+ -mwindows -lntdll
	$(STRIP) --strip-unneeded $@

autowpakey.exe: autowpakey.o wlankey.o resource.o tea.o libwlanapi.a
	$(CC) -o $@ $+ -mconsole -lntdll
	$(STRIP) --strip-unneeded $@

mkwlankeys.exe: mkwlankeys.o tea.o
	$(CC) -o $@ $+ -lkernel32 -lntdll
	$(STRIP) --strip-unneeded $@

getwpaconf.exe: getwpaconf.o libwlanapi.a
	$(CC) -o $@ $+ -mconsole -lntdll
	$(STRIP) --strip-unneeded $@

setwpakey.o: wlankey.h tea.h

wpakeygui.o: wlankey.h tea.h dialog.h

autowpakey.o: wlankey.h tea.h

wlankey.o: wlankeys.h wlankey.h tea.h

mkwlankeys.o: wlankey.h tea.h

tea.o: tea.h

resource.o: resource.rc
	$(WINDRES) -o $@ $+

dialog.o: dialog.rc
	$(WINDRES) -o $@ $+

libwlanapi.a: wlanapi.def
	$(DLLTOOL) --as=$(AS) -k --output-lib $@ --def $<
