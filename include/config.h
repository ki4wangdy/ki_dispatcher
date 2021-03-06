
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define buf_max_size	100

#include "platform.h"

struct config_st{

	// module_im_server push ip and pull ip
	int8_t imserver_push_ip_addr[buf_max_size];
	int8_t imserver_pull_ip_addr[buf_max_size];

	int8_t schat_push_ip_addr[buf_max_size];
	int8_t schat_pull_ip_addr[buf_max_size];

	// memcached server
	int8_t memcacheq_server [buf_max_size];
	int32_t memcacheq_port;
	
	int8_t imserver_ip[buf_max_size];
	int8_t schat_topic[buf_max_size];

};

config_t config_init();

void config_destory(config_t);

#endif