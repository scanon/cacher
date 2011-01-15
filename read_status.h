#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

struct cache_file_t {
  char *filename;
  key_t key;
  int id;   
  size_t size;
  void *ptr;
};


int read_status(char *file,struct cache_file_t *cf, int max);
