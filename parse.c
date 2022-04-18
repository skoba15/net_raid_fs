
#include <string.h>
#include "parse.h"




void tokenize(char * ss, storage * st){
	int i;
	int num = 0;
	for(i=0; i<strlen(ss); i++){
		st->servers[num] = malloc(16);
		memcpy(st->servers[num], (char *)ss+i, 15);
		st->servers[num][15]='\0';
		i+=16;
		num++;
	}
	st->serversNum = num;
}


int parser(char * path, config * result){
	FILE * f;
	f = fopen(path, "r");
	if(f==NULL){
		return -1;
	}

	char str[200];
	result->errorlog = malloc(512);
	result->cache_replacement = malloc(512);
	result->storagesNum = 5;
	result->storages = malloc(sizeof(storage)*result->storagesNum);
	result->loglen = 0;
	fscanf(f, "errorlog = %s cache_size = %dM cache_replacement = %s timeout = %d", 
		result->errorlog, &result->cacheSize, result->cache_replacement, &result->timeout);
	//printf("%s\n%d\n%s\n%d\n", result->errorlog, result->cacheSize, result->cache_replacement, result->timeout);
	storage * cur = malloc(sizeof(storage));
	cur->diskname = malloc(512);
	cur->mountpoint = malloc(512);
	cur->hotswap = malloc(512);
	cur->servers = malloc(sizeof(char *)*100);
	char * ss=malloc(512);
	while(fscanf(f, " diskname = %s mountpoint = %s raid = %d servers = %[^\n] hotswap = %s ",
	 cur->diskname, cur->mountpoint, &cur->raid, ss, cur->hotswap)!=EOF){
		tokenize(ss, cur);
		if(result->loglen == result->storagesNum){
			result->storagesNum *=2;
			result->storages = realloc(result->storages, result->storagesNum*sizeof(storage));
		}

		result->storages[result->loglen].diskname = cur->diskname;
		result->storages[result->loglen].mountpoint = cur->mountpoint;
		result->storages[result->loglen].raid = cur->raid;
		result->storages[result->loglen].servers = cur->servers;
		result->storages[result->loglen].hotswap = cur->hotswap;
		result->storages[result->loglen].serversNum = cur->serversNum;
		free(ss);
		cur = malloc(sizeof(storage));
		cur->diskname = malloc(512);
		cur->mountpoint = malloc(512);
		cur->hotswap = malloc(512);
		cur->servers = malloc(sizeof(char *)*100);
		ss=malloc(512);
		result->loglen++;
	}
	
	// int i;
	// for(i=0; i<2; i++){
	// 	printf("%s\n", result->storages[i].diskname);
	// 	printf("%s\n", result->storages[i].mountpoint);
	// 	printf("%d\n", result->storages[i].raid);
	// 	printf("%s\n", result->storages[i].servers[1]);
	// 	printf("%s\n", result->storages[i].hotswap);
	// 	printf("%d\n", result->storages[i].serversNum);
	// }

	fclose(f);
	return 1;
}


void destroyStorage(storage * st){
	free(st->diskname);
	free(st->mountpoint);
	free(st->hotswap);
	int i;
	for(i=0; i<st->serversNum; i++){
		free(st->servers[i]);
	}
	free(st->servers);
}


void destroyConfig(config * c){
	int i;
	for(i=0; i<c->loglen; i++){
		destroyStorage(&c->storages[i]);
	}
	free(c->errorlog);
	free(c->cache_replacement);
	free(c->storages);
	free(c);
}


// int main(){
// 	parser("config");
// 	return 0;
// }