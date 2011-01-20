
all: cacher libtrap.so

cacher: cacher.o

libtrap.so: trap_so.c
	rm -f libtrap.so*
	gcc -fPIC -shared -Wl,-soname,libtrap.so.1 -o libtrap.so.1.0  trap_so.c -ldl
	ln -s libtrap.so.1.0 libtrap.so.1
	ln -s libtrap.so.1 libtrap.so

clean:
	rm -f libtrap.so* *.o cacher
