

#include <fuse.h>
#include <dirent.h>
#include <semaphore.h>

struct inf{
  char path[PATH_MAX];
  struct stat statbuf;
  mode_t mode;
  dev_t dev;
  char newpath[500];
  off_t newsize;
  struct utimbuf utibuf;
  struct fuse_file_info fi;
  size_t size;
  off_t offset;
  fuse_fill_dir_t filler;
  DIR * pointer;
  int fh;
  int syscallnum;
  int flags;
  int delete;
  int damaged;
  int status;
};

struct attrinfo{
  struct stat result;
  int retstat;
};


struct openstr{
  int fd;
  int res;
};


 struct str{
  struct stat statbuf;
  int retstat;
}str;


struct opend{
  DIR * a;
  int retstat;
};


struct readd{
  char * buf;
  int sz;
  int retstat;
};





struct mp{
  char name[PATH_MAX];
  int fd;
};




struct stInfo{
  char errorlog[256];
  char diskname[256];
  int fds[2];
  char servers[2][256];
  int fd;
  struct cache * c;
  struct mp * maps;
  int loglen;
  int alloclen;
  char hotswap[256];
  sem_t s;
  int numDead;
  int timeout;
  FILE * log;
};


struct threadInfo{
  struct stInfo * st;
  int sfdInd;
  int waiting;
};




