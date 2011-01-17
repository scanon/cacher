#include "read_status.h"


int read_status(char *file,struct cache_file_t *cf,int max)
{
  FILE *f;
  char buffer[1024];
  char **pos;
  char *running;
  char *t;
  int i;

   f=fopen(file,"r");
   if (f==NULL){
     perror("fopen");
     return 0;
   }
   i=0;
   while(fscanf(f,"%s",buffer)>0 && i<max){
     running = strdup (buffer);
     t=strsep(&running,":");
     cf[i].key=atoi(t);
     t=strsep(&running,":");
     cf[i].filename=strdup(t);
     t=strsep(&running,":");
     t=strsep(&running,":");
     cf[i].size=atol(running);
     i++;
   }
   cf[i].filename=NULL;
   fclose(f);   
}
