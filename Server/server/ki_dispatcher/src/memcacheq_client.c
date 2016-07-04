

#include "platform.h"
#include "memcacheq.h"
#include <libmemcached/memcached.h>

static memcached_st* memc;

// init the memcacheq 
int memcacheq_init(char* server, int port){
	memcached_return rc;
	memcached_server_st* servers = NULL;
	memc = memcached_create(NULL);
	servers = memcached_server_list_append(server, server, port, &rc);
	rc = memcached_server_push(memc, servers);

	if (MEMCACHED_SUCCESS != rc){
		fprintf(stderr, "memcached_server_push failed!\n");
	}
	memcached_server_list_free(servers);
	return 4;
}

// 1  for success
// -1 for error
int memcacheq_set(int fd, char* topic, char* value, int value_len){
	int32_t flags = 0;
	memcached_return rc;
	rc = memcached_set(memc, key, strlen(key), value, strlen(value), 0, flags);
	return MEMCACHED_SUCCESS == rc ? 1 : -1;
}

// 1  for has message
// 0  for nothing
// -1 for error 
int memcacheq_get(int fd, char* topic, char** value, int* len){

	int32_t flags = 0;
	memcached_return rc;
	int value_length;
	char* rvalue = memcached_get(memc, topic, strlen(topic), &value_length, &flags, &rc);
	*value = rvalue;
	*len = value_length;
	if (len == 0 || strcmp(rvalue,"") == 0){
		return 0;
	}
	return rc == MEMCACHED_SUCCESS ? 1 : -1;
}

// close the connect
int memcacheq_close(int fd){
	memcached_free(memc);
}