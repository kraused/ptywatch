
VERSION    = 1
PATCHLEVEL = 0

CC       = gcc
CPPFLAGS = -I. -D_GNU_SOURCE
CCFLAGS  = -Wall -O0 -ggdb -fPIC
LD       = gcc
LDFLAGS  = -O0 -ggdb -fPIC -Wl,-export-dynamic
LIBS     = -lutil -lutempter -ldl

Q = @

default: all

all: ptywatch.exe plugins/stdout.so plugins/libnotify.so dbus-signal/dbus-signal.so

ptywatch.exe: ptywatch.o plugin.o error.o
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "LD $@"

plugins/libnotify.o: plugins/libnotify.c
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) `pkg-config --cflags glib-2.0` `pkg-config --cflags gdk-pixbuf-2.0` -o $@ -c $<
	@echo "CC $@"

plugins/libnotify.so: plugins/libnotify.o error.o
	$(Q)$(CC) $(LDFLAGS) `pkg-config --libs glib-2.0` -lnotify -shared -o $@ $<
	@echo "CC $@"

dbus-signal/dbus-signal.so:
	make -C dbus-signal

%.o: %.c
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) -o $@ -c $<
	@echo "CC $@"

%.so: %.o error.o
	$(Q)$(CC) $(LDFLAGS) -shared -o $@ $<
	@echo "CC $@"

tar:
	python2 tar.py ptywatch $(VERSION).$(PATCHLEVEL)

install:
	install -m0755 -d $(PREFIX)/usr/sbin/
	install -m0755 -d $(PREFIX)/usr/libexec/ptywatch/
	install -m0755 -d $(PREFIX)/usr/lib/systemd/system/
	install -m0755 ptywatch.exe		$(PREFIX)/usr/sbin/ptywatch.exe
	install -m0755 plugins/libnotify.so	$(PREFIX)/usr/libexec/ptywatch/libnotify.so
	install -m0644 ptywatch.service		$(PREFIX)/usr/lib/systemd/system/ptywatch.service
	make -C dbus-signal PREFIX=$(PREFIX) install

clean:
	-rm -f *.o plugins/*.o plugins/*.so ptywatch.exe

