#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "read_status.h"

void * (*next_mmap)(void *start, size_t length, int prot, int flags, int fd, off_t offset);
void * (*next_mmap64)(void *__addr, size_t __len, int __prot, int __flags, int __fd, __off64_t __offset);
int (*next_munmap)(void *start, size_t length);
void init(void);
void *lookup_file(int fd,struct cache_file_t *cf);
int check_ptr(void *ptr);

#define MAX_CACHE 20
struct cache_file_t cf[MAX_CACHE];

void * mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  void *p=NULL;

//  fprintf(stderr,"mmap %lx %ld %ld %d\n",(long)start,length,offset,prot);
  if (next_mmap == NULL)
    init();
  if (prot==PROT_READ){
    p=lookup_file(fd,cf);
  }
  if (p==NULL)
    return next_mmap(start, length, prot, flags, fd, offset);
  else
    return p;
}

void * mmap64(void *start, size_t length, int prot, int flags, int fd, __off64_t offset)
{
  void *p=NULL;

//  fprintf(stderr,"mmap64 %lx %ld %ld %d\n",(long)start,length,offset,prot);
  if (next_mmap64 == NULL)
    init();
  if (prot==PROT_READ){
    p=lookup_file(fd,cf);
  }
  if (p==NULL)
    return next_mmap64(start, length, prot, flags, fd, offset);
  else
    return p;
}
int munmap(void *start, size_t length)
{
//  fprintf(stderr,"munmap %lx %ld\n",(long)start,length);
  if (next_mmap64 == NULL)
    init();
  if (check_ptr(start)){
//    fprintf(stderr,"DEBUG: Skipping %lx\n",start);
    return 1;
  }
  else
    return next_munmap(start, length);
}

void init(void)
{
  char *file;
  int shm_id;
  struct shm_info si;
  struct shmid_ds ss;
  int i,j,k=0;
  void *ptr;
 
  shmctl(0,SHM_INFO,(struct shmid_ds *)&si);
  for (k=0;k<si.used_ids;k++){
    j=shmctl(k,SHM_STAT,&ss);
    if (j>0){
     printf("%d\n",j);
     cf[i].key=ss.shm_perm.__key;
     shm_id = shmget(cf[i].key, cf[i].size, 0666);
     if (shm_id < 0) {
        fprintf(stderr,"*** shmget error cacher ***\n");
        exit(1);
     }
     ptr = (void *) shmat(shm_id, NULL, 0);
     if ((long) ptr == -1) { /* attach */
          fprintf(stderr,"*** shmat error cacher ***\n");
          exit(1);
     }
     cf[i].filename=(char *)ptr;
     cf[i].ptr=ptr+strlen((char *)ptr)+1;
    i++;
    }
  }
/*
  while (i<MAX_CACHE && cf[i].filename!=NULL){
     }
//     fprintf(stderr,"ptr=%lx\n",(long)cf[i].ptr);
  }*/
  next_mmap64 = dlsym(RTLD_NEXT, "mmap64");
  next_mmap = dlsym(RTLD_NEXT, "mmap");
  next_munmap = dlsym(RTLD_NEXT, "munmap");
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
//  fprintf(stderr,"DEBUG: %s %s\n",buffer,buff2);
  i=0;
  while (i<MAX_CACHE && cf[i].filename!=NULL){
   if (strstr(buff2,cf[i].filename)!=0){
//     fprintf(stderr,"found ptr 0x%lx\n",(long)cf[i].ptr);
     return cf[i].ptr;
   } 
   i++;
  } 
  return NULL;
}

int check_ptr(void *start)
{
  int i=0;
  while (i<MAX_CACHE && cf[i].filename!=NULL){
    if (cf[i].ptr==start){
      return 1;
    }
    i++;
  }
  return 0;
}
