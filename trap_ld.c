#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dlfcn.h>

void * (*__real_mmap)(void *start, size_t length, int prot, int flags, int fd, off_t offset);

void * __wrap_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  printf("mmap %ld %d\n",start,length);
  return (void *)__real_mmap(*start, length, prot, flags, fd, offset);
}


