
CC       = gcc
CPPFLAGS = -I. -I.. -D_GNU_SOURCE
CCFLAGS  = -Wall -O0 -ggdb `pkg-config --cflags gio-2.0` `pkg-config --cflags gio-unix-2.0` `pkg-config --cflags glib-2.0`
LD       = gcc
LDFLAGS  = -O0 -ggdb
LIBS     = `pkg-config --libs gio-2.0` `pkg-config --libs gio-unix-2.0` `pkg-config --libs glib-2.0` 

Q = @

default: all

all: notifyd.exe

notifyd.exe: notifications.o notifyd.o ../error.o
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "LD  $@"

%.o: %.c
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) -o $@ -c $<
	@echo "CC $@"

notifications.c: org.freedesktop.Notifications.xml
	gdbus-codegen --generate-c-code notifications --interface-prefix org.freedesktop $<

clean:
	-rm -f *.o ptywatch.exe

