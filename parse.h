#include <stdio.h>
#include <stdlib.h>
#include "assert.h"

typedef struct{
	char * diskname;
	char * mountpoint;
	int raid;
	char ** servers;
	int serversNum;
	char * hotswap;
}storage;


typedef struct{
	char * errorlog;
	int cacheSize;
	char * cache_replacement;
	int timeout;
	int storagesNum;
	int loglen;
	storage * storages;
}config;




int parser(char * path, config * c);