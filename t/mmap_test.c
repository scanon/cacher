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
   char* ch;
   char buff[1024];
   char *p;
   struct stat st;
   long size;
   int fd=open(argv[1],O_RDONLY);

   fstat(fd,&st);
   size=st.st_size;
   p=(void *)mmap(0,size,PROT_READ,MAP_PRIVATE,fd,0L);
   if ((long)p==-1){
     perror("mmap failed\n");
     exit;
   }
   write(1,p,size);
   exit;
   
}
