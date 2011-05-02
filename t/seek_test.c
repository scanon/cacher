#include <stdlib.h>
#include <stdio.h>
#include <sys/fcntl.h>

#include <sys/shm.h>
#include <string.h>
#include <sys/mman.h>
 #include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_CACHE 20

 
int main(int argc, char *argv[])
{
   char buff[1024];
   size_t size;
   int fd;
   off_t off;

   if (argc<1) return -1;
   fd=open(argv[1],O_RDONLY);
   off=lseek(fd,1,SEEK_SET);
   fprintf(stderr,"lseek offset=%ld\n",off);
   while((size=read(fd,buff,1024))>0){
     fprintf(stderr,"return=%d\n",size);
     write(1,buff,size);
   }
   fprintf(stderr,"return=%d\n",size);
   exit;
   
   
}
