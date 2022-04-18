#ifndef MY_CACHE
#define MY_CACHE

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
	char path[1000];
	size_t size;
	off_t offset;
	void * file;
	struct node * prev;
	struct node * next;
};


struct cache {
	struct node * start;
	struct node * tail;
	int used;
	int max;
};


void printCache(struct cache * c);
int rename_nodes(struct cache * c, char * path, char * newname);
int create_new_node(struct cache * c, struct node * prev, struct node * next, char * path, size_t size, off_t offset, char * buf);
void create_cache(struct cache * c, int max_memory);
int add_node(struct cache * c, char * path, off_t offset, size_t size, char * buf);
int remove_node(struct cache * c, char * path);
#endif