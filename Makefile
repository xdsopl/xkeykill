CFLAGS = -std=c99 -W -Wall -O3 -D_POSIX_C_SOURCE
LDFLAGS = -lxcb -lxcb-keysyms

xkeykill: xkeykill.o strkeysym.o

strkeysym.o: strkeysym.c keysymdef.h strkeysym.h

keysymdef.h: /usr/include/X11/keysymdef.h
	grep '^#define XK_' $< | sed 's/^#define\ XK_\([a-zA-Z_0-9]*\)[ ]*\(0x[a-fA-F0-9]*\)[ ]*\(.*\)/{ \"\1\", \2 },\3/g' > $@

test: xkeykill
	./xkeykill "<Alt>F4" xclock

clean:
	rm -f xkeykill *.o keysymdef.h

