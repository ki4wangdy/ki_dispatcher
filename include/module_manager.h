
#ifndef _MODULE_MANAGER_H_
#define _MODULE_MANAGER_H_

#include <chash.h>
#include <config.h>

typedef struct module_manager_st{
	pthread_mutex_t lock;
	struct hash_table* hash;
	struct config_st* config;
	void* zmq_context;
};

module_manager_t module_manager_init(config_t);
void module_manager_destory(module_manager_t m);

void module_manager_notify(module_manager_t m, const char* module_flag);
void module_manager_add_module(module_manager_t m, const char* module_flag, module_t module);

#endif