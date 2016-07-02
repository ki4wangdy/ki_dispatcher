
#ifndef _MODULE_MANAGER_H_
#define _MODULE_MANAGER_H_

typedef struct module_manager_st{
	pthread_mutex_t lock;
	hash_table_t hash;
	config_t config;
	void* zmq_context;
}*module_manager_t;

module_manager_t module_manager_init(config_t);
void module_manager_destory(module_manager_t m);

void module_manager_notify(module_manager_t m, const char* module_flag);
void module_manager_add_module(module_manager_t m, const char* module_flag, module_t module);

#endif