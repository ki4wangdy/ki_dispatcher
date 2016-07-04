
#include "config.h"

config_t config_init(){

	config_t t = r_calloc(1,sizeof(struct config_st));

	// init the imserver_push ip
	strcpy(t->imserver_push_ip_addr,"tcp://121.42.207.163:22222");

	// init the imserver_pull ip
	strcpy(t->imserver_pull_ip_addr,"tcp://121.42.207.163:22221");

	// init the schat push ip
	strcpy(t->schat_push_ip_addr,"tcp://121.42.207.163:22226");

	// init the schat pull ip
	strcpy(t->schat_pull_ip_addr,"tcp://121.42.207.163:22225");

	// init the imserver ip
	strcpy(t->imserver_ip,"121.42.207.163");

	// init the memcacheq ip
	strcpy(t->memcacheq_server, "121.42.207.163");
	t->memcacheq_port = 11212;

	// init the schat topic
	strcpy(t->schat_topic,"schat_module");

	return t;
}

void config_destory(config_t t){
	r_free(t);
}