
all: cacher libtrap.so

cacher: cacher.o

PREFIX?=/usr/local
BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/lib

libtrap.so: trap_so.c
	rm -f libtrap.so*
	gcc -fPIC -shared -Wl,-soname,libtrap.so.1 -o libtrap.so.1.0  trap_so.c -ldl
	ln -s libtrap.so.1.0 libtrap.so.1
	ln -s libtrap.so.1 libtrap.so

install: cacher libtrap.so
	mkdir -p $(BINDIR)
	mkdir -p $(LIBDIR)
	install -t $(BINDIR) cacher
	install -t $(LIBDIR) libtrap.so*
clean:
	rm -f libtrap.so* *.o cacher
