
TESTS=mmap_test thread_test read_test seek_test

all: $(TESTS)
#CFLAGS=-lpthread

mmap_test: mmap_test.o

test:
	LD_PRELOAD=../libtrap.so ./mmap_test /etc/hosts
	LD_PRELOAD=../libtrap.so ./read_test /etc/hosts
	LD_PRELOAD=../libtrap.so ./thread_test /etc/hosts
clean:
	rm -f *.o $(TESTS)
