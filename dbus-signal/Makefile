
CC       = gcc
CPPFLAGS = -I. -I.. -D_GNU_SOURCE -DNDEBUG=1
CCFLAGS  = -Wall -O0 -ggdb -fPIC `pkg-config --cflags gio-2.0` `pkg-config --cflags gio-unix-2.0` `pkg-config --cflags glib-2.0`
LD       = gcc
LDFLAGS  = -O0 -ggdb -fPIC -Wl,-export-dynamic
LIBS     = `pkg-config --libs gio-2.0` `pkg-config --libs gio-unix-2.0` `pkg-config --libs glib-2.0` -lpthread

Q = @

default: all

all: dbus-signal.so wait4signal.exe

dbus-signal.so: ptywatch-dbus-signal.o dbus-signal.o ../error.o
	$(Q)$(CC) $(LDFLAGS) -shared -o $@ $^ $(LIBS)
	@echo "LD $@"

wait4signal.exe: wait4signal.o ../error.o
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "LD $@"

%.o: %.c
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) -o $@ -c $<
	@echo "CC $@"

ptywatch-dbus-signal.c: org.freedesktop.PtyWatchDbusSignal.xml
	gdbus-codegen --generate-c-code ptywatch-dbus-signal --interface-prefix org.freedesktop $<

install:
	install -m0755 -d $(PREFIX)/usr/bin/
	install -m0755 -d $(PREFIX)/usr/libexec/ptywatch/
	install -m0755 wait4signal.exe	$(PREFIX)/usr/bin/wait4signal.exe
	install -m0755 dbus-signal.so	$(PREFIX)/usr/libexec/ptywatch/dbus-signal.so

clean:
	-rm -f *.o dbus-signal.so wait4signal.exe

