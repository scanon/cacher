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
#include <time.h>
 
#define SHM_KEY 1098
#define CHUNK 1048576L
 
size_t cache_file(char *file,int key);
void cleanup();
void status();

int debug=0;

int main(int argc, char *argv[])
{
   int i;
   int key;
   char *name;
   time_t start;
   size_t total=0;

   start=time(NULL);
   if (getenv("DEBUG_CACHER")!=NULL)
     debug=1;
   if (argc==1){
     status();
     return 0;
   }
   if (argv[1][0]=='-'){
     cleanup();
     return 0;
   }
   for (key=SHM_KEY,i=1;i<argc;i++,key++){
      total+=cache_file(argv[i],key); 
   }
   if (debug)
     fprintf(stderr,"Total: %ld bytes in %ld seconds (%7.2f MB/s)\n",total,time(NULL)-start,(double)total/(double)(1024.0*1024.0*(time(NULL)-start)));
   return 0;
}

size_t cache_file(char *file,int key)
{
   int shmid;
   void *ptr;
   int size;
   int fd;
   size_t br;
   size_t total;
   struct stat st;
   time_t start;


   start=time(NULL);   
   fd=open(file,O_RDONLY);
   fstat(fd,&st);
   size=st.st_size;
   shmid=shmget(key, size+strlen(file)+1, IPC_CREAT|0600);
   if( shmid == -1){
      perror("shmget");
      return 0;
    }
   ptr = (char*)shmat(shmid, 0, 0);
   if((long)ptr == -1L) {
      shmctl(shmid, IPC_RMID, NULL);
      perror("shmget");
      return 0;
   }
   strcpy(ptr,file);
   ptr+=strlen(file)+1;
   total=0;
   while( (br=read(fd,ptr,CHUNK))>0){
     ptr+=br;
     total+=br;
   }
   shmdt(ptr);
   if (debug)
     fprintf(stderr,"%-70s: cached %10ld bytes in %3d seconds\n",file,total,time(NULL)-start);

   return total;
}

void cleanup()
{
  struct shm_info si;
  struct shmid_ds ss;
  int i,j;
  if (debug)
    fprintf(stderr,"Cleanup\n");  

  shmctl(0,SHM_INFO,(struct shmid_ds *)&si);
  for (i=0;i<si.used_ids;i++){
    j=shmctl(i,SHM_STAT,&ss);
    if (j>0){
      shmctl(j, IPC_RMID, NULL);
    } 
  }
}

void status()
{
  struct shm_info si;
  struct shmid_ds ss;
  int i,j;
  char *ptr;
  key_t key;
  size_t size;
  int shm_id;

  shmctl(0,SHM_INFO,(struct shmid_ds *)&si);
  for (i=0;i<si.used_ids;i++){
    j=shmctl(i,SHM_STAT,&ss);
    if (j>0 && ss.shm_perm.uid==getuid()){
     key=ss.shm_perm.__key;
     size=ss.shm_segsz;   
     shm_id = shmget(key, size, 0600);
     if (shm_id < 0) {
        fprintf(stderr,"*** shmget error cacher ***\n");
        exit(1);
     }
     ptr = (void *) shmat(j, NULL, 0);
     if ((long) ptr == -1) { /* attach */
          fprintf(stderr,"*** shmat error cacher ***\n");
          exit(1);
     }
      printf("0x%08x: %s\n",ss.shm_perm.__key,ptr);
    } 
  }
}
