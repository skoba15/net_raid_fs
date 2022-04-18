#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#include "util.h"
#include "parse.h"
#include <pthread.h>
// for socket api
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

#include "cache.h"







int nrf_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);



int client(char * srv, char * prt, int sfd_i, struct stInfo * st);


///////////////////////////////////////////////////////////






/** Get file attributes */
int cl_getattr(const char *path, struct stat *statbuf)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  inform.syscallnum = -1;
  send(st->fds[0], &inform, sizeof(struct inf), 0);
  struct attrinfo atr;
  recv(st->fds[0], &atr, sizeof(struct attrinfo), 0);
  memcpy(statbuf, &(atr.result), sizeof(struct stat));
    
  return atr.retstat;
}


/** Create a file node */
int cl_mknod(const char *path, mode_t mode, dev_t dev)
{
  struct inf inform;
  strcpy(inform.path, path);
  inform.mode = mode;
  inform.dev = dev;
  inform.syscallnum = 0;
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
	

    

    
    sem_wait(&st->s);
    send(st->fds[0], &inform, sizeof(struct inf), 0);
    int res;
    recv(st->fds[0], &res, sizeof(int), 0);

    send(st->fds[1], &inform, sizeof(struct inf), 0);
    int r;
    recv(st->fds[1], &r, sizeof(int), 0);
    sem_post(&st->s);
    return res;
}

/** Create a directory */
int cl_mkdir(const char *path, mode_t mode)
{

struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
 
  struct inf inform;
  strcpy(inform.path, path);
  inform.mode = mode;
  inform.syscallnum = 1;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);
	

 int retstat;
 recv(st->fds[0], &retstat, sizeof(int), 0);
 int r2;
 send(st->fds[1], &inform, sizeof(struct inf), 0);
 recv(st->fds[1], &r2, sizeof(int), 0);
  sem_post(&st->s);
 return retstat;
}

/** Remove a file */
int cl_unlink(const char *path)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
   remove_node(st->c, (char *)path);
  struct inf inform;
  strcpy(inform.path, path);
  inform.syscallnum = 2;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);
  int retstat;
  recv(st->fds[0], &retstat, sizeof(int), 0);
  int r2;
  send(st->fds[1], &inform, sizeof(struct inf), 0);
  recv(st->fds[1], &r2, sizeof(int), 0);
   sem_post(&st->s);
  return retstat;
}

/** Remove a directory */
int cl_rmdir(const char *path)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  inform.syscallnum = 3;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);
  int retstat;
  recv(st->fds[0], &retstat, sizeof(int), 0);
  int r2;
  send(st->fds[1], &inform, sizeof(struct inf), 0);
  recv(st->fds[1], &r2, sizeof(int), 0);
   sem_post(&st->s);
  return retstat;
}

/** Rename a file */
// both path and newpath are fs-relative
int cl_rename(const char *path, const char *newpath)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  strcpy(inform.newpath, newpath);
  inform.syscallnum = 4;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);

  int retstat;
  recv(st->fds[0], &retstat, sizeof(int), 0);
   int r2;
  send(st->fds[1], &inform, sizeof(struct inf), 0);
  recv(st->fds[1], &r2, sizeof(int), 0);
   sem_post(&st->s);
  return retstat;
}

/** Change the size of a file */
int cl_truncate(const char *path, off_t newsize)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  inform.newsize = newsize;
  inform.syscallnum = 5;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);

	int retstat;
  recv(st->fds[0], &retstat, sizeof(int), 0);
   int r2;
  send(st->fds[1], &inform, sizeof(struct inf), 0);
  recv(st->fds[1], &r2, sizeof(int), 0);
   sem_post(&st->s);
  return retstat;
}









int cl_open(const char *path, struct fuse_file_info *fi)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  inform.flags = fi->flags;
  inform.syscallnum = 7;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);

    int fd;
    int retstat = 0;

	  struct openstr op;
    recv(st->fds[0], &op, sizeof(struct openstr), 0);
    char old_hash[33];
    char new_hash[33];
    recv(st->fds[0], old_hash, 33, 0);
    recv(st->fds[0], new_hash, 33, 0);
   
    fi->fh = op.fd;

    send(st->fds[1], &inform, sizeof(struct inf), 0);
    struct openstr op2;
    recv(st->fds[1], &op2, sizeof(struct openstr), 0);

    char old_hash1[33];
    char new_hash1[33];
	recv(st->fds[1], old_hash1, 33, 0);
    recv(st->fds[1], new_hash1, 33, 0);



     int i;
     fd =-1;
  for(i=0; i<st->loglen; i++){
    if(strcmp(st->maps[i].name, path)==0){
      fd = st->maps[i].fd;
    }
  }


  if(fd == -1){
    if(st->loglen == st->alloclen){
      st->alloclen*=2;
      st->maps = realloc(st->maps, st->alloclen*sizeof(struct mp));
    }
    strcpy(st->maps[st->loglen].name, path);
    st->maps[st->loglen].fd = op2.fd;
    st->loglen++;
  }
  else{
    for(i=0; i<st->loglen; i++){
      if(strcmp(st->maps[i].name, path)==0){
        st->maps[i].fd = op2.fd;
      }
    }
  }






      
     
    if(strcmp(old_hash, new_hash)==0 && strcmp(old_hash1, new_hash1)==0){
    	
    }else if(strcmp(old_hash, new_hash)==0 && strcmp(old_hash1, new_hash1)!=0){
    	



      char * str1 = "reading data from the first server for stable storage purposes";
      logging_message(str1, st->log, st->servers[0], st->diskname);

      char * str2 = "writing data from the first server to the second one for stable storage purposes";
      logging_message(str2, st->log, st->servers[1],  st->diskname);



    	int len = 0;
      while(1){
        char buf[4096];
        struct fuse_file_info ff;
        ff.fh = fi->fh;


        
        struct inf inform2;
        strcpy(inform2.path, path);
        inform2.size = 4096;
        inform2.offset = len;
        inform2.syscallnum = 8;
        inform2.fh = ff.fh;
        send(st->fds[0], &inform2, sizeof(struct inf), 0);
        
       
       
        int retstat;
   
        recv(st->fds[0], buf, 4096, 0);
        recv(st->fds[0], &retstat, sizeof(int), 0);

        int res = retstat;




        
        if(res == 0){
          buf[0]='\0';
          res = 1;
        }
        struct inf inform1;
        strcpy(inform1.path, path);
        inform1.size = res;
        inform1.offset = len;
        inform1.syscallnum = 9;
        inform1.fh = op2.fd;
        inform1.damaged = 1;
         if(len == 0){
            inform1.delete = 1;
          }
          else{
            inform1.delete = 0;
          }
        if(res < 4096){
          send(st->fds[1], &inform1, sizeof(struct inf), 0);
          send(st->fds[1], buf, res, 0);

          int r2;
          recv(st->fds[1], &r2, sizeof(int), 0);
          break;
        }
        send(st->fds[1], &inform1, sizeof(struct inf), 0);
        send(st->fds[1], buf, res, 0);
        int r2;
        recv(st->fds[1], &r2, sizeof(int), 0);
        len+=res;
      }
    }else if(strcmp(old_hash, new_hash)!=0 && strcmp(old_hash1, new_hash1)==0){
    	int len = 0;


      char * str1 = "reading data from the second server for stable storage purposes";
      logging_message(str1, st->log, st->servers[1], st->diskname);

      char * str2 = "writing data from the second server to the first one for stable storage purposes";
      logging_message(str2, st->log, st->servers[0], st->diskname);
    	while(1){
    		char buf[4096];
    		struct fuse_file_info ff;
    		ff.fh = op2.fd;


			  struct inf inform2;
			  strcpy(inform2.path, path);
			  inform2.size = 4096;
			  inform2.offset = len;
			  inform2.syscallnum = 8;
			  inform2.fh = ff.fh;
			  send(st->fds[1], &inform2, sizeof(struct inf), 0);
			  
			 
				
			  int retstat;
			  recv(st->fds[1], buf, 4096, 0);
			  recv(st->fds[1], &retstat, sizeof(int), 0);

			  int res = retstat;




    		
    		if(res == 0){
    			buf[0]='\0';
          res = 1;
    		}
		    struct inf inform1;
		    strcpy(inform1.path, path);
		    inform1.size = res;
		    inform1.offset = len;
		    inform1.syscallnum = 9;
		    inform1.fh = op.fd;
		    inform1.damaged = 1;
		     if(len == 0){
    				inform1.delete = 1;
    			}
    			else{
    				inform1.delete = 0;
    			}
    		if(res < 4096){
			    send(st->fds[0], &inform1, sizeof(struct inf), 0);
			    send(st->fds[0], buf, res, 0);
			    int r2;
			    recv(st->fds[0], &r2, sizeof(int), 0);
    			break;
    		}
    		send(st->fds[0], &inform1, sizeof(struct inf), 0);
		    send(st->fds[0], buf, res, 0);
		    int r2;
		    recv(st->fds[0], &r2, sizeof(int), 0);
		    len+=res;
    	}
    }else{

      char * str1 = "deleting data from the first server for the stable storage purposes";
      logging_message(str1, st->log, st->servers[0], st->diskname);

      char * str2 = "deleting data from the second server for the stable storage purposes";
      logging_message(str2, st->log, st->servers[1], st->diskname);



      struct inf inform;
      strcpy(inform.path, path);
      inform.syscallnum = 2;

    	send(st->fds[0], &inform, sizeof(struct inf), 0);
      int retstat;
      recv(st->fds[0], &retstat, sizeof(int), 0);
      int r2;
      send(st->fds[1], &inform, sizeof(struct inf), 0);
      recv(st->fds[1], &r2, sizeof(int), 0);
    }
     sem_post(&st->s);
    return op.res;
}

/** Read data from an open file */
int cl_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  inform.size = size;
  inform.offset = offset;
  inform.syscallnum = 8;
  inform.fh = fi->fh;
   sem_wait(&st->s);
  if(add_node(st->c, (char *)path, offset, size, buf)==1){
  

    char * str = "getting data to cache";
    logging_message(str, st->log, st->servers[0], st->diskname);
    // printCache(st->c);
     sem_post(&st->s);
    return size;
  }
  else{
    send(st->fds[0], &inform, sizeof(struct inf), 0);

    int retstat;

    recv(st->fds[0], buf, size, 0);
    recv(st->fds[0], &retstat, sizeof(int), 0);
    if(size<=st->c->max){
      char * str = "adding data to cache";
      logging_message(str, st->log, st->servers[0], st->diskname);
      create_new_node(st->c, st->c->tail->prev, st->c->tail, (char *)path, size, offset, buf);
    } 
     sem_post(&st->s);
    return retstat;
  }
}
  

/** Write data to an open file */
int cl_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
    struct inf inform;
    strcpy(inform.path, path);
    inform.size = size;
    inform.offset = offset;
    inform.syscallnum = 9;
    inform.fh = fi->fh;
    inform.delete = 0;
    inform.damaged = 0;
    
     sem_wait(&st->s);
    remove_node(st->c, (char *)path);

    
    send(st->fds[0], &inform, sizeof(struct inf), 0);
    send(st->fds[0], buf, size, 0);
    int retstat;
    recv(st->fds[0], &retstat, sizeof(int), 0);
    
    int i;
    int fd = -1;
    for(i=0; i<st->loglen; i++){
      if(strcmp(st->maps[i].name, path)==0){
        fd = st->maps[i].fd;
      }
    }
    inform.fh = fd;

    send(st->fds[1], &inform, sizeof(struct inf), 0);
    send(st->fds[1], buf, size, 0);
    int r2;
    recv(st->fds[1], &r2, sizeof(int), 0);
    // printCache(st->c);
     sem_post(&st->s);
    return retstat;
}


/** Release an open file */
int cl_release(const char *path, struct fuse_file_info *fi)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  inform.fh=fi->fh;
  inform.syscallnum = 10;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);
  int retstat;
  recv(st->fds[0], &retstat, sizeof(int), 0);

  send(st->fds[1], &inform, sizeof(struct inf), 0);
  int retstat2;
  recv(st->fds[1], &retstat2, sizeof(int), 0);
   sem_post(&st->s);
  return retstat;
}


int cl_utime(const char *path, struct utimbuf *ubuf)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  return 0;
}

/** Open directory */
int cl_opendir(const char *path, struct fuse_file_info *fi)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  memcpy(&inform.fi, fi, sizeof(struct fuse_file_info));
  inform.syscallnum = 11;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);

    struct opend op;
    recv(st->fds[0], &op, sizeof(struct opend), 0);
    fi->fh = (intptr_t) (op.a);
     sem_post(&st->s);
    return op.retstat;
}

/** Read directory */
int cl_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  inform.pointer = (DIR *)(uintptr_t)(fi->fh);
  inform.offset = offset;
  inform.syscallnum = 12;
   sem_wait(&st->s);
  
  send(st->fds[0], &inform, sizeof(struct inf), 0);
    int sz;
    
    recv(st->fds[0], &sz, sizeof(int), 0);
    char buff[sz];
    recv(st->fds[0], buff, sz, 0);
    char * p = strtok(buff, "/");
    do {
    	if (filler(buf, p, NULL, 0) != 0) {
    	    return -ENOMEM;
    	}
    } while ((p = strtok(NULL, "/")) != NULL);
    int retstat;
    recv(st->fds[0], &retstat, sizeof(int), 0);
     sem_post(&st->s);
    return retstat;
}

/** Release directory */
int cl_releasedir(const char *path, struct fuse_file_info *fi)
{
  struct stInfo * st = (struct stInfo *)fuse_get_context()->private_data;
  struct inf inform;
  strcpy(inform.path, path);
  inform.pointer = (DIR *)(uintptr_t)fi->fh;
  inform.syscallnum = 13;
   sem_wait(&st->s);
  send(st->fds[0], &inform, sizeof(struct inf), 0);
  int retstat;
  recv(st->fds[0], &retstat, sizeof(int), 0);
   sem_post(&st->s);
  return 0;
}

struct fuse_operations nrf_oper = {
  .getattr = cl_getattr,
  .mknod = cl_mknod,
  .mkdir = cl_mkdir,
  .unlink = cl_unlink,
  .rmdir = cl_rmdir,
  .rename = cl_rename,
  .truncate = cl_truncate,
  .utime = cl_utime,
  .open = cl_open,
  .read = cl_read,
  .write = cl_write,
  .opendir = cl_opendir,
  .readdir = cl_readdir,
};







void connectagain(char * str, int id, int var, struct stInfo * st, struct threadInfo * tinf){
  if(var==1){
    char * server = strdup(str);
    char * srv = strtok(server, ":");
    char * prt = strtok(NULL, ":");
    int port = atoi(prt);
    struct sockaddr_in addr;
    int ip;
    char buf[1024];
    int num1 = socket(AF_INET, SOCK_STREAM, 0);


    // sfd_arr[0] = sfd;
    inet_pton(AF_INET, srv, &ip);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip;


    int num = connect(num1, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

    if(num<0){
      
      return;
    }
    else{
      char * msg = "hotswap server added";
      logging_message(msg, tinf->st->log,  str, tinf->st->diskname);
      tinf->waiting = 0;
      st->numDead++;
      st->fds[id] = num1;
    }
    
  }
  else{
    char * server = st->servers[id];
    char * cp = strdup(server);
    char * srv = strtok(cp, ":");
    char * prt = strtok(NULL, ":");
    struct sockaddr_in addr;
    int ip;
    char buf[1024];
    int num1 = socket(AF_INET, SOCK_STREAM, 0);

    
    // sfd_arr[0] = sfd;
    inet_pton(AF_INET, srv, &ip);
    int port = atoi(prt); //atoi (token);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip;



    int num = connect(num1, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

    if(num<0){
      return;
    }
    else{
      char * msg = "open connection";
      logging_message(msg, tinf->st->log,  tinf->st->servers[tinf->sfdInd], tinf->st->diskname);
      tinf->waiting = 0;
      st->fds[id] = num1;
    }
    
  }
}




int logging_message(char* msg, FILE* logfile,  char* server, char* diskname){
  time_t currenttime;
  char * str = malloc(128);

  currenttime = time ( NULL );
  str = ctime(&currenttime);
  int l = strlen(str) - 1;
  str[l] = ' ';
  char buf[500];

  strcpy(buf, "[");
  strcat(buf, str);
  strcat(buf, "]");
  strcat(buf, diskname);
  strcat(buf, "  ");
  strcat(buf, server);
  strcat(buf, "  ");
  strcat(buf, msg);
  fprintf(logfile, "%s\n", buf);
  fflush(logfile);
}






void * startThreads(void * in){
  struct threadInfo * tinf = (struct threadInfo *)in;
  tinf->waiting = 0;
  time_t start;
  time_t end;
  while(1){
   int dead = 1;
    struct inf info;
    info.syscallnum = 14;
    int sfd = tinf->st->fds[tinf->sfdInd];
    sem_wait(&tinf->st->s);
    send(sfd, &info, sizeof(struct inf), 0);
    recv(sfd, &dead, sizeof(int), 0);

    if(dead == 1){
      if(tinf->waiting == 1){
        time(&end);
        int passedTime = difftime(end, start);
        if(passedTime >= tinf->st->timeout){
          char * str = "server declared as lost";

          logging_message(str, tinf->st->log,  tinf->st->servers[tinf->sfdInd], tinf->st->diskname);
          int ind= 0;
         
          connectagain(tinf->st->hotswap, tinf->sfdInd, 1, tinf->st, tinf);
           if(tinf->sfdInd==0){
            ind = 1;
          }
          
        }
        else{
           connectagain(" ", tinf->sfdInd, 0, tinf->st, tinf);
        }
      }
      else{

        tinf->waiting = 1;
        time(&start);
      }
    }
    sem_post(&tinf->st->s);
    sleep(1);
  }
  
}

int main(int argc, char *argv[])
{

	config * c= malloc(sizeof(config));

	int argv_n = 2;
	if(argc == 2)
		argv_n = 1;

	int res = parser(argv[2], c);

   
  int i;
  int status;
  int j;
  int wpid;
  FILE * log = fopen(c->errorlog, "a+");
  for(j = 0; j< c->storagesNum; j++){
    int pid = fork();
    if(pid == 0){
      struct stInfo * st = malloc(sizeof(struct stInfo));
      strcpy(st->errorlog, c->errorlog);
      strcpy(st->diskname, c->storages[j].diskname);
      st->loglen = 0;
      st->alloclen = 5;
      strcpy(st->hotswap, c->storages[j].hotswap);
      st->maps = malloc(st->alloclen * sizeof(struct mp));
      st->numDead = 0;
      st->log = log;
      st->c = malloc(sizeof(struct cache));
      create_cache(st->c, c->cacheSize * 1024 * 1024);
      char * argvCur[3];
       argvCur[0] = malloc(100);
       argvCur[1] = malloc(100);
       argvCur[2] = malloc(100);
       strcpy(argvCur[0], argv[0]);
       strcpy(argvCur[1], argv[1]);
       strcpy(argvCur[2], argv[2]);
      strcpy(argvCur[2], c->storages[j].mountpoint);
      sem_init(&st->s, 0, 1);
      st->timeout = c->timeout;
       for(i = 0; i < c->storages[j].serversNum; i++){
        strcpy(st->servers[i], c->storages[j].servers[i]);
        char * srv = strtok(c->storages[j].servers[i], ":");
        char * prt = strtok(NULL, ":");
        client(srv, prt, i, st);
        pthread_t th;
        
        struct threadInfo * tinf=malloc(sizeof(struct threadInfo));
        tinf->st = st;
        tinf->sfdInd = i;
        pthread_create(&th, NULL, startThreads, tinf);
      }

     
      fuse_main( argc, argvCur, &nrf_oper, st);
      break;
    }
  }
 

	
  while((wpid = wait(&status)) > 0);
}








//client side scratch
int client(char * srv, char * prt, int sfd_i, struct stInfo * st){

  struct sockaddr_in addr;
  int ip;
  char buf[1024];
  st->fds[sfd_i] = socket(AF_INET, SOCK_STREAM, 0);
  // sfd_arr[0] = sfd;
  inet_pton(AF_INET, srv, &ip);



  int port = atoi(prt);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip;


  connect(st->fds[sfd_i], (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
        char * msg = "open connection";
      logging_message(msg, st->log,  st->servers[sfd_i], st->diskname);
}