
CC       = gcc
CPPFLAGS = -I. -D_GNU_SOURCE
CCFLAGS  = -Wall -O0 -ggdb
LD       = gcc
LDFLAGS  = -O0 -ggdb
LIBS     = -lutil -lutempter

OBJ   = ptywatch.o error.o

Q = @

default: all

all: ptywatch.exe

ptywatch.exe: $(OBJ)
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "LD  $@"

%.o: %.c
	$(Q)$(CC) $(CPPFLAGS) $(CCFLAGS) -o $@ -c $<
	@echo "CC $@"


clean:
	-rm -f *.o ptywatch.exe

