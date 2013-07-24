CFLAGS = -std=c99 -W -Wall -O3 -D_POSIX_C_SOURCE
LDFLAGS = -lxcb -lxcb-keysyms

xkeykill: xkeykill.o strkeysym.o

test: xkeykill
	./xkeykill "<Alt>F4" xclock

clean:
	rm -f xkeykill *.o

