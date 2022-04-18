#include "cache.h"


void create_cache(struct cache * c, int max_memory) {
	c->used = 0;
	c->max = max_memory;
	c->start = malloc(sizeof(struct node));
	c->tail = malloc(sizeof(struct node));
	c->start->next = c->tail;
	c->start->prev = NULL;
	c->tail->prev = c->start;
	c->tail->next = NULL;
}




int create_new_node(struct cache * c, struct node * prev, struct node * next, char * path, size_t size, off_t offset, char * buf){
	struct node * new_node = malloc(sizeof(struct node));
	new_node->prev = prev;
	new_node->next = next;
	strcpy(new_node->path, path);
	new_node->size = size;
	
	new_node->offset = offset;
	new_node->file=malloc(size);
	memcpy((char*)(new_node->file), buf, size);
	
	prev->next = new_node;
	next->prev = new_node;
	c->used = c->used + size;
	return 0;
}




void printCache(struct cache * c){
	struct node * cur = c->start->next;
	printf("%s\n", "_____________START______________");
	while(cur->next!=NULL){
		printf("PATH %s SIZE %d OFFSET %d FILE INFO: %s\n", cur->path, (int)cur->size, (int)cur->offset, (char *)cur->file);
		cur = cur->next;
	}
	printf("%s\n", "_________________END___________________");
}


int rename_nodes(struct cache * c, char * path, char * newname){
	struct node * cur = c->start->next;
	while(cur->next!=NULL){
		if(strcmp(cur->path, path)==0){
			strcpy(cur->path, newname);
		}
		cur=cur->next;
	}
}


int add_node(struct cache * c, char * path, off_t offset, size_t size, char * buf){
	struct node * cur = c->start->next;
	while(c->used + size > c->max && cur->next!=NULL){
		c->used = c->used - cur->size;
		cur->next->prev = c->start;
		c->start->next = cur->next;
		cur = cur->next;
	}
	cur = c->start->next;
	while(cur->next!=NULL){
		if(strcmp(cur->path, path)==0 && cur->offset<=offset && cur->offset + cur->size >= offset + size){
			int i;
			memcpy(buf, (char *)cur->file+offset-cur->offset, size);
			cur->prev->next = cur->next;
			cur->next->prev = cur->prev;
			cur->next = c->tail;
			cur->prev = c->tail->prev;
			c->tail->prev = cur;
			cur->prev->next = cur;
			return 1;
		}
		cur = cur->next;
	}
	return 0;
}



int remove_node(struct cache * c, char * path){
	struct node * cur = c->start->next;
	while(cur->next!=NULL){
		if(strcmp(cur->path, path)==0){
			cur->prev->next = cur->next;
			cur->next->prev = cur->prev;
			c->used = c->used - cur->size;
		}
		cur=cur->next;
	}
	//printCache(c);
	return 0;
}