#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dlfcn.h>
#include "read_status.h"

void * (*next_mmap64)(void *start, size_t length, int prot, int flags, int fd, off_t offset);
void * (*next_mmap)(void *start, size_t length, int prot, int flags, int fd, off_t offset);
void init(void);
void *lookup_file(int fd,struct cache_file_t *cf);

#define MAX_CACHE 20
struct cache_file_t cf[MAX_CACHE];

void * mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  void *p;

  if (next_mmap == NULL)
    init();
  p=lookup_file(fd,cf);
  if (p==NULL)
    return next_mmap(start, length, prot, flags, fd, offset);
  else
    return p;
}

void * mmap64(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{

  fprintf(stderr,"mmap %ld %d\n",(long)start,length);
  if (next_mmap64 == NULL)
    init();
  return next_mmap64(start, length, prot, flags, fd, offset);
}

void init(void)
{
  char *file;
  file=getenv("CACHER");
  if (file!=NULL){
    printf("Loading hack.\n");
    read_status(file,cf,MAX_CACHE);
  }
  next_mmap64 = dlsym(RTLD_NEXT, "mmap64");
  next_mmap = dlsym(RTLD_NEXT, "mmap");
}

void * lookup_file(int fd,struct cache_file_t *cf)
{
  char buffer[1024];
  char buff2[1024];
  int shm_id;
  int *shm_ptr;
  int i;

  strcpy(buffer,"/proc/self/fd/");
  sprintf(buffer,"/proc/self/fd/%d",fd);
  i=readlink(buffer,buff2,1024);
  buff2[i]=0;
  printf("%s %s\n",buffer,buff2);
  i=0;
  while (i<MAX_CACHE && cf[i].filename!=NULL){
   printf("%s\n",cf[i].filename);
   if (strstr(buff2,cf[i].filename)!=0){
     printf("found key %d\n", cf[i].key);
     shm_id = shmget(cf[i].key, cf[i].size, 0666);
     if (shm_id < 0) {
        printf("*** shmget error (client) ***\n");
        exit(1);
     }
     shm_ptr = (int *) shmat(shm_id, NULL, 0);
     if ((long) shm_ptr == -1) { /* attach */
          printf("*** shmat error (client) ***\n");
          exit(1);
     }
     return shm_ptr;
   } 
   i++;
  } 
  return NULL;
}
