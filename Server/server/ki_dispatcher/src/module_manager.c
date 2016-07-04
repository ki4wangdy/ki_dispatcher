
#include "platform.h"

module_manager_t module_manager_init(config_t config){

	module_manager_t m = r_calloc(1,sizeof(struct module_manager_st));
	m->config = config;
	m->hash = make_nocase_string_hash_table(10);
	pthread_mutex_init(&m->lock,NULL);
	m->zmq_context = zmq_ctx_new();
	return m;

}

void module_manager_destory(module_manager_t m){
	assert(m != NULL);

	// destory the config
	m->config = NULL;

	// destroy the lock
	pthread_mutex_unlock(&m->lock);

	// find the imserver module and destroy it
	module_t imserver_module = hash_table_get(m->hash,module_flag_imserver);
	if(imserver_module != NULL){
		imserver_module->module_destory();
	}

	// find the schat module and destroy it
	module_t schat_module = hash_table_get(m->hash,module_flag_single_chat);
	if(schat_module != NULL){
		schat_module->module_destory();
	}

	// for each the item, then destroy the item, final destory the hash
	hash_table_destroy(m->hash);

}

void module_manager_notify(module_manager_t m, const char* module_flag){
	assert(m != NULL);
	pthread_mutex_lock(&m->lock);
	module_t module = (module_t)hash_table_get(m->hash,module_flag);
	if(module != NULL){
		module->module_notify_pull();
	}
	pthread_mutex_unlock(&m->lock);
}

void module_manager_add_module(module_manager_t m, const char* module_flag, module_t module){
	assert(m != NULL);
	hash_table_put(m->hash,module_flag,module);
}

void module_manager_start_all(module_manager_t manager){
	// 1. start the im server module
	module_t imserver_module = (module_t)hash_table_get(manager->hash,module_flag_imserver);
	if(imserver_module != NULL){
		imserver_module->module_start();
	}

	// 2. start the schat module
	module_t schat_module = (module_t)hash_table_get(manager->hash,module_flag_single_chat);
	if(schat_module != NULL){
		schat_module->module_start();
	}
	
}

module_t module_manager_get_module(module_manager_t manager, char* module_flag){
	assert(manager);
	return (module_t)hash_table_get(manager->hash,module_flag);
}


