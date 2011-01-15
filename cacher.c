#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


#include <sys/shm.h>
#define _GNU_SOURCE
#include <string.h>
 
#define NAME "CACHER"
#define SHM_KEY 1098
 
void *cache_file(char *file,int key,FILE *f);
void cleanup();

struct cache_file {
  void *ptr; 
  int id;
  struct cache_file *next;
};

struct cache_file *head;
struct cache_file *tail;

int main(int argc, char *argv[])
{
   int i;
   int key;
   FILE *f;

   f=fopen(getenv(NAME),"w");
   if (f==NULL){
     perror("fopen");
     exit;
   }
   for (key=SHM_KEY,i=1;i<argc;i++,key++){
    cache_file(argv[i],key,f); 
   }
   fclose(f);

   while(1){
     sleep(5);
     cleanup();
     exit;
   }
}

void *cache_file(char *file,int key,FILE *f)
{
   int shmid;
   void *ptr;
   int size;
   int fd;
   struct stat st;


   fd=open(file,O_RDONLY);
   fstat(fd,&st);
   size=st.st_size;
   shmid=shmget(key, size, IPC_CREAT|0666);
   if( shmid == -1){
      perror("shmget");
      exit(1);
    }
   ptr = (char*)shmat(shmid, 0, 0);
   if((long)ptr == -1L) {
      shmctl(shmid, IPC_RMID, NULL);
      perror("shmget");
      exit(1);
   }
   read(fd,ptr,size);
   fprintf(f,"%d:%s:%ld:%d:%d\n",key,file,ptr,shmid,size);

   return ptr;
}

void cleanup()
{
  
}
