#include <stdlib.h>
#include <stdio.h>
#include <sys/fcntl.h>

#include <sys/shm.h>
#include <string.h>
#include <sys/mman.h>
#include "read_status.h"

#define MAX_CACHE 20

 
int main()
{
   char* ch;
   char buff[1024];
   char *p;
   int fd=open("/etc/hosts",O_RDONLY);

   p=(void *)mmap(0,1024L,PROT_READ,MAP_PRIVATE,fd,0L);
   if ((long)p==-1){
     perror("mmap failed\n");
     exit;
   }
   printf("p=%llx\n",p);
   memcpy(buff,p,10); 
   buff[11]=0;
   printf("%s\n",buff);
   exit;
   
}
