#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <stdlib.h>
#include "util.h"
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <utime.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <sys/xattr.h>
#include <sys/epoll.h>



#define BACKLOG 10







int hash(char * filename, char* hash){
    int MD_DIGEST_LENGTH = 16;
    unsigned char c[MD_DIGEST_LENGTH];
    int i;
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
        return 0;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);

    MD5_Final (c,&mdContext);

    char md5string[33];
    
    for(i = 0; i < 16; ++i)
      sprintf(&md5string[i*2], "%02x", (unsigned int)c[i]);

    strcpy(hash, md5string);
    

    fclose (inFile);
    return 0;
}


int globalsfd;


char * storagedir;

static void nrf_fullpath(char fullpath[PATH_MAX], const char *path)
{
    strcpy(fullpath, storagedir);
    strncat(fullpath, path, PATH_MAX); 
}


int main(int argc, char* argv[]){

  

  int sfd, cfd;
  struct sockaddr_in addr;
  struct sockaddr_in peer_addr;
  int port = atoi(argv[2]);


  sfd = socket(AF_INET, SOCK_STREAM, 0);
  globalsfd = sfd;
  int optval = 1;
  setsockopt(sfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&optval,sizeof(optval));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
  listen(sfd, BACKLOG);
  int peer_addr_size = sizeof(struct sockaddr_in);
  

  cfd = accept(sfd,(struct sockaddr *) &peer_addr, &peer_addr_size);
  
  int buf[1024];
  int data_size;
  

  storagedir = malloc(256);

  strcpy(storagedir, argv[3]);
  
  int length = 512;

  int epoll_fd = epoll_create(1);
  struct epoll_event e;
  struct epoll_event es[1];
  e.events = EPOLLIN;
  e.data.fd = cfd;
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cfd, &e);


while(1){



  int ready = epoll_wait(epoll_fd, es, 1, -1);
  int i = 0;
  int curcfd=0;
  for(; i < ready; i++){
      int curcfd = es[i].data.fd;



  char revbuf[length]; // Receiver buffer
  struct inf res;
  recv(curcfd, &res, sizeof(struct inf), 0);


  if(res.syscallnum==14){
    
    int aa = 0;
    send(curcfd, &aa, sizeof(int), 0);
  }

  
  if(res.syscallnum == -1){
    
    char fullpath[PATH_MAX];
    nrf_fullpath(fullpath, res.path);
    struct stat * statbuf=malloc(sizeof(struct stat));
    struct attrinfo atr;
    int retstat = lstat(fullpath, statbuf);
    memcpy(&(atr.result), statbuf, sizeof(struct stat));
    if(retstat < 0)
      retstat = -errno; 
    atr.retstat = retstat;
    send(curcfd, &atr, sizeof(struct attrinfo), 0);
}else if(res.syscallnum == 0){
    int retstat;
    char fullpath[PATH_MAX];
    nrf_fullpath(fullpath, res.path);

    // hash
   
    

    
    retstat = open(fullpath, O_CREAT | O_EXCL | O_WRONLY, res.mode);

    if (retstat >= 0){
        retstat = close(retstat);
    }
      
    if(retstat < 0){
      retstat = -errno;
    }
    char file_hash[33];
    hash(fullpath, file_hash);
    setxattr(fullpath, "user.hash", file_hash, 33, 0);
    send(curcfd, &retstat, sizeof(int), 0);
}else if(res.syscallnum == 1){
   
    char fullpath[PATH_MAX];
    nrf_fullpath(fullpath, res.path);
    int retstat = mkdir(fullpath, res.mode);
    if(retstat < 0)
      retstat = -errno;
    send(curcfd, &retstat, sizeof(int), 0);
  }
  else if(res.syscallnum == 2){
   
    char fullpath[PATH_MAX];
    nrf_fullpath(fullpath, res.path);
    int retstat = unlink(fullpath);
    if(retstat < 0)
      retstat = -errno;
     send(curcfd, &retstat, sizeof(int), 0);
  }
  else if(res.syscallnum == 3){
  
    char fullpath[PATH_MAX];
    nrf_fullpath(fullpath, res.path);

    int retstat = rmdir(fullpath);
    if(retstat < 0)
      retstat = -errno;
    send(curcfd, &retstat, sizeof(int), 0);
   }
   else if(res.syscallnum == 4){
      

      char fullpath[PATH_MAX];
      char fnewpath[PATH_MAX];
      nrf_fullpath(fullpath, res.path);
      nrf_fullpath(fnewpath, res.newpath);
      int retstat = rename(fullpath, fnewpath);
      if(retstat < 0)
        retstat = -errno;
      char file_hash[33];
      // hash(fnewpath, file_hash);
      // setxattr(fnewpath, "user.hash", file_hash, 33, 0);
      send(curcfd, &retstat, sizeof(int), 0);
    }else if(res.syscallnum == 5){

      char fullpath[PATH_MAX];
      nrf_fullpath(fullpath, res.path);
      
      int retstat = truncate(fullpath, res.newsize);
      if(retstat < 0)
        retstat = -errno;
      char file_hash[33];
      send(curcfd, &retstat, sizeof(int), 0);
  }
  
  else if(res.syscallnum == 7){


    int fd;
    int retstat = 0;
    char fullpath[PATH_MAX];
    nrf_fullpath(fullpath, res.path);
    struct openstr op;
    fd = open(fullpath, res.flags);
      if (fd < 0){
        fd = -errno;
        retstat = -errno;
      }
     op.fd = fd;
     op.res = retstat;
     char old_hash[33];
     char new_hash[33];
      getxattr(fullpath, "user.hash", old_hash, 33);
     hash(fullpath, new_hash);
     send(curcfd, &op, sizeof(struct openstr), 0);
     send(curcfd, old_hash, 33, 0);
     send(curcfd, new_hash, 33, 0);
}
  
  else if(res.syscallnum == 8){
      char buf[res.size];
      int retstat = pread(res.fh, buf, res.size, res.offset);
      if(retstat < 0){
        retstat = -errno;
      }

      send(curcfd, buf, res.size, 0);
      send(curcfd, &retstat, sizeof(int), 0);
  }
  else if(res.syscallnum == 9){
      char fullpath[PATH_MAX];
    nrf_fullpath(fullpath, res.path);
     char buf[res.size];
     recv(curcfd, buf, res.size, 0);
    int fdd = 0;
    int retstat = 0;
    if(res.damaged==1){
  
       FILE * fop;
       if(res.delete == 1){
          fop = fopen(fullpath, "w");
       
          retstat = fwrite(buf, 1, res.size, fop);
          fclose(fop);
       }
       else{
        
          fop = fopen(fullpath, "a");
          retstat = fwrite(buf, 1, res.size, fop);
          fclose(fop);
       }
    }
    else{
      
      retstat = pwrite(res.fh, buf, res.size, res.offset);
    }
   

    ;
  

    if(retstat < 0)
      retstat = -errno;

    
    
    char file_hash[33];
    hash(fullpath, file_hash);
    setxattr(fullpath, "user.hash", file_hash, 33, 0);
    send(curcfd, &retstat, sizeof(int), 0);
}
  else if(res.syscallnum == 10){
    
      int retstat = close(res.fh);
    if(retstat < 0)
      retstat = -errno;

    send(curcfd, &retstat, sizeof(int), 0);
  }
  else if(res.syscallnum == 11){
    DIR *dp;
    int retstat = 0;
    char fullpath[PATH_MAX];
    
    nrf_fullpath(fullpath, res.path);

   
    dp = opendir(fullpath);
    struct opend op;
    op.a=dp;
    if (dp == NULL)
      retstat = -errno;
    op.retstat = retstat;
    
    send(curcfd, &op, sizeof(struct opend), 0);
  }
  else if(res.syscallnum == 12){
   
    int retstat = 0;
    DIR *dp;
    struct dirent *de;
    
    dp = res.pointer;

    de = readdir(dp);
    

    int sz = 0;
    int n = 0;
    int len = 0;
    char res[10000];
    res[0] = '\0';
    if (de == 0) {
     retstat = -errno;
    }
    else{
      while (1){
          if(de==NULL){
            break;
          }
        strcat(res, de->d_name);
        strcat(res, "/");
        de = readdir(dp);
      }
    }
   
    
    sz = strlen(res)+1;
    send(curcfd, &sz, sizeof(int), 0);
    send(curcfd, res, sz, 0);
    send(curcfd, &retstat, sizeof(int), 0);
}else if(res.syscallnum == 13){
      int retstat = 0;

    
      closedir(res.pointer);

    
    send(curcfd, &retstat, sizeof(int), 0);
 }
  
  }
}
  close(cfd);
  close(sfd);
}