#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/resource.h>


struct cache_file_t {
  char *filename;
  key_t key;
  int id;   
  size_t size;
  void *ptr;
};

struct cache_fd {
  int status;
  off_t off;
  struct cache_file_t *cache; 
};

void * (*next_mmap)(void *start, size_t length, int prot, int flags, int fd, off_t offset);
void * (*next_mmap64)(void *__addr, size_t __len, int __prot, int __flags, int __fd, __off64_t __offset);
int (*next_munmap)(void *start, size_t length);
int (*next_open)(const char *pathname, int flags, mode_t mode);
ssize_t (*next_read)(int fd, void *buf, size_t count);
size_t (*next_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
off_t (*next_lseek)(int fd, off_t offset, int whence);
int (*next_fseek)(FILE *stream, off_t offset, int whence);
int (*next_close)(int fd);
int (*next_fclose)(FILE *stream);
void init(void);
struct cache_file_t *lookup_file(int fd,struct cache_file_t *cf);
void *lookup_ptr(int fd,struct cache_file_t *cf);
int check_ptr(void *ptr);
int iscached(int fd);

#define MAX_CACHE 20

struct cache_file_t cf[MAX_CACHE];
int debug=0;
struct cache_fd *cachefd;
int max_fd;

int pdebug(const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  if (debug)
    return vfprintf(stderr,format,ap);
}

inline int iscached(fd)
{
  struct cache_file_t *c=NULL;

  pdebug("iscache: checking fd=%d\n",fd);

  if (fd<max_fd){     
    if (cachefd[fd].status==1){  // Mapped and cached
      return 1;
    }
    else if (cachefd[fd].status==0){  // Not Mapped yet
      pdebug("iscache: lookup fd=%d\n",fd);
      c=lookup_file(fd,cf);
      if (c!=NULL){
        pdebug("iscache: lookup success for fd=%d %s\n",fd,c->filename);
        cachefd[fd].status=1;
        cachefd[fd].off=0;
        cachefd[fd].cache=c;
        return 1;
      }
      else{ 
        cachefd[fd].status=2;
        return 0;
      }
    } 
    else{  // Mapped but not cached
      return 0;
    }
  }
  return 0;
}


int open3(const char *pathname, int flags, mode_t mode)
{

  if (next_open == NULL)
    init();
  pdebug("trapped open: %s\n",pathname);
  return next_open(pathname,flags,mode);
}

off_t lseek(int fd, off_t offset, int whence)
{
  struct cache_file_t *c=NULL;
  if (next_mmap == NULL)
    init();
  pdebug("lseek: trapped fd=%d offset=%ld\n",fd,offset);
  if (iscached(fd)){ 
    c=cachefd[fd].cache;
    if (whence==SEEK_SET)
      cachefd[fd].off=offset;
    else if (whence==SEEK_CUR)
      cachefd[fd].off=cachefd[fd].off+offset;
    else if (whence==SEEK_END)
      cachefd[fd].off=c->size+offset;
    else
      return EINVAL;

    pdebug("lseek: cached fd=%d offset=%d size=%ld\n",fd,cachefd[fd].off,c->size);
    if (cachefd[fd].off < 0 ){
      cachefd[fd].off=0;
      return EINVAL;
    }
    return cachefd[fd].off;
  }
  else{
    return next_lseek(fd,offset,whence);
  }
}

int fseek(FILE *stream, off_t offset, int whence)
{
  int fd=fileno(stream);
  if (next_mmap == NULL)
    init();
  pdebug("fseek: trapped fd=%d offset=%ld\n",fd,offset);
  if (iscached(fd))
    return (lseek(fd,offset,whence));
  else
    return next_fseek(stream,offset,whence);
}

ssize_t read(int fd, void *buf, size_t count)
{
  struct cache_file_t *c=NULL;

  if (next_mmap == NULL)
    init();
  pdebug("read: trapped fd=%d count=%ld\n",fd,count);

    
  if (iscached(fd)){ 
    c=cachefd[fd].cache;
    pdebug("read: cached fd=%d offset=%d size=%ld\n",fd,cachefd[fd].off,c->size);
    if ((cachefd[fd].off+count)>(c->size)){
      count=(c->size-cachefd[fd].off);
    }
    pdebug("read: cached fd=%d count=%d\n",fd,count);
    memcpy(buf,cachefd[fd].cache->ptr+cachefd[fd].off,count);
    cachefd[fd].off+=count;
    return count;
  }
  else{
    return next_read(fd,buf,count);
  }
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  int fd=fileno(stream);

  if (next_mmap == NULL)
    init();
  pdebug("fread: trapped fd=%d count=%ld\n",fd,size);
  if (iscached(fd))
    return (read(fd,ptr,size*nmemb)/size);
  else
    return next_fread(ptr,size,nmemb,stream);
}

int close(int fd)
{
  pdebug("close: trapped fd=%d\n",fd);
  if (fd<max_fd)cachefd[fd].status=0;
  return next_close(fd);
}

int fclose(FILE *stream)
{
  int fd=fileno(stream);
  pdebug("close: trapped fd=%d\n",fd);
  if (fd<max_fd)cachefd[fd].status=0;
  return next_fclose(stream);
}

void * mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  void *p=NULL;

  if (next_mmap == NULL)
    init();
  pdebug("trapped mmap: start=0x%lx length=%ld %ld\n",(long)start,length,offset);
  if (prot==PROT_READ){
    p=lookup_ptr(fd,cf);
  }
  if (p==NULL)
    return next_mmap(start, length, prot, flags, fd, offset);
  else
    return p;
}

void * mmap64(void *start, size_t length, int prot, int flags, int fd, __off64_t offset)
{
  void *p=NULL;

    pdebug("trapped mmap64: start=0x%lx %ld %ld %d\n",(long)start,length,offset,prot);
  if (next_mmap64 == NULL)
    init();
  if (prot==PROT_READ){
    p=lookup_ptr(fd,cf);
  }
  if (p==NULL)
    return next_mmap64(start, length, prot, flags, fd, offset);
  else
    return p;
}
int munmap(void *start, size_t length)
{
  pdebug("trapped munmap: 0x%lx %ld\n",(long)start,length);
  if (next_mmap64 == NULL)
    init();
  if (check_ptr(start)){
    pdebug("DEBUG: Skipping %lx\n",start);
    return 1;
  }
  else
    return next_munmap(start, length);
}

void init(void) __attribute__((constructor));
void init(void)
{
  char *file;
  int shm_id;
  struct shm_info si;
  struct shmid_ds ss;
  int i,j,k=0;
  void *ptr;
  int resource;
  struct rlimit rlim;

  if (getenv("DEBUG_CACHER")!=NULL){
    debug=1; 
    pdebug("Init: DEBUG Enabled\n");
  }
  pdebug("Init: Clearing cachefd\n");
  getrlimit(RLIMIT_NOFILE, &rlim);
  pdebug("max files %d\n",rlim.rlim_cur);
  max_fd=rlim.rlim_cur;
  cachefd=(struct cache_fd *)malloc(max_fd*sizeof(struct cache_fd));
  if (cachefd==NULL){
    fprintf(stderr,"Failed to alloc cache fd\n");
    max_fd=0;
  }
  for (i=0;i<max_fd;i++){cachefd[i].status=0;}
  pdebug("Init: Initialzing shared memory\n");
  shmctl(0,SHM_INFO,(struct shmid_ds *)&si);
  i=0;
  for (k=0;k<si.used_ids;k++){
    pdebug("Init: Shmctl for %ld out of %d\n",k,si.used_ids);
    j=shmctl(k,SHM_STAT,&ss);
    pdebug("Init: Owner for %d uid=%d\n",k,ss.shm_perm.uid);
    if (j>=0 && ss.shm_perm.uid==getuid()){
     cf[i].key=ss.shm_perm.__key;
     cf[i].size=ss.shm_segsz;
    pdebug("Init: Shmget for %d\n",cf[i].key);
     shm_id = shmget(cf[i].key, ss.shm_segsz, 0666);
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
     pdebug("Init: Cacher %s\n",ptr);
     cf[i].ptr=ptr+strlen((char *)ptr)+1;
     cf[i].size-=(strlen((char *)ptr)+1);
    i++;
    }
  }
  pdebug("Init: Mapping functions\n");
  next_mmap64 = dlsym(RTLD_NEXT, "mmap64");
  next_mmap = dlsym(RTLD_NEXT, "mmap");
  next_munmap = dlsym(RTLD_NEXT, "munmap");
  next_open = dlsym(RTLD_NEXT, "open");
  next_read = dlsym(RTLD_NEXT, "read");
  next_fread = dlsym(RTLD_NEXT, "fread");
  next_lseek = dlsym(RTLD_NEXT, "lseek");
  next_fseek = dlsym(RTLD_NEXT, "fseek");
  next_close = dlsym(RTLD_NEXT, "close");
  next_fclose = dlsym(RTLD_NEXT, "fclose");
}

struct cache_file_t * lookup_file(int fd,struct cache_file_t *cf)
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
  pdebug("lookup: %s %s\n",buffer,buff2);
  i=0;
  while (i<MAX_CACHE && cf[i].filename!=NULL){
   if (strcmp(buff2,cf[i].filename)==0){
     pdebug("lookup: found ptr 0x%lx\n",(long)cf[i].ptr);
     return &cf[i];
   } 
   i++;
  } 
  return NULL;
}

void * lookup_ptr(int fd,struct cache_file_t *cf)
{
   struct cache_file_t *c;
   c=lookup_file(fd,cf);
   if (c!=NULL)
     return c->ptr;
   else
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

