
#include "config.h"

init_config config_init(){

	config_t t = r_calloc(1,sizeof(struct config_st));

	// init the imserver_push ip
	strcpy(t->imserver_push_ip_addr,"127.0.0.1");

	// init the imserver_pull ip
	strcpy(t->imserver_pull_ip_addr,"127.0.0.1");

	// init the schat push ip
	strcpy(t->schat_push_ip_addr,"127.0.0.1");

	// init the schat pull ip
	strcpy(t->schat_pull_ip_addr,"127.0.0.1");

	// init the imserver ip
	strcpy(t->imserver_ip,"127.0.0.1");

	// init the schat topic
	strcpy(t->schat_topic,"schat_module");

	return t;
}

void config_destory(config_t t){
	s_free(t);
}