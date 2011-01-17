#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/shm.h>
#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
 
#define SHM_KEY 1098
 
void *cache_file(char *file,int key);
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
   char *name;

   if (argv[1][0]=='-'){
     cleanup();
     return 0;
   }
   for (key=SHM_KEY,i=1;i<argc;i++,key++){
    cache_file(argv[i],key); 
   }
   return 0;
}

void *cache_file(char *file,int key)
{
   int shmid;
   void *ptr;
   int size;
   int fd;
   int br;
   struct stat st;
   struct cache_file *c;


   fd=open(file,O_RDONLY);
   fstat(fd,&st);
   size=st.st_size;
   shmid=shmget(key, size+strlen(file)+1, IPC_CREAT|0666);
   if( shmid == -1){
      perror("shmget");
      return NULL;
    }
   ptr = (char*)shmat(shmid, 0, 0);
   if((long)ptr == -1L) {
      shmctl(shmid, IPC_RMID, NULL);
      perror("shmget");
      return NULL;
   }
   strcpy(ptr,file);
   ptr+=strlen(file)+1;
   br=read(fd,ptr,size);
   printf("bytes read: %ld\n",br);
//   fprintf(f,"%d:%s:%ld:%d:%d\n",key,file,ptr,shmid,size);
   c=malloc(sizeof(struct cache_file));
   c->id=shmid;
   c->ptr=ptr;
   if (head==NULL){
     head=c;
   }
   else{
     tail->next=c;
   }
   tail=c;

   return ptr;
}

void cleanup()
{
  struct shm_info si;
  struct shmid_ds ss;
  int i,j;
  printf("Cleanup\n");  

  shmctl(0,SHM_INFO,(struct shmid_ds *)&si);
  for (i=0;i<si.used_ids;i++){
    j=shmctl(i,SHM_STAT,&ss);
    if (j>0){
      shmctl(j, IPC_RMID, NULL);
    } 
  }
}
