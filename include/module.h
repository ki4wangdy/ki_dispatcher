
#ifndef _MODULE_H_
#define _MODULE_H_

#include "platform.h"

#define module_flag_imserver		"im_server"
#define module_flag_router			"router"
#define module_flag_single_chat		"schat"
#define module_flag_mutil_chat		"mchat"

struct module_st{
	void (*module_init)(module_manager_t);
	void (*module_start)();
	void (*module_notify_pull)();
	void (*module_pull_process)(int8_t*);
	void (*module_push_process)(int8_t*);
	void (*module_destory)();
};

void module_inits(module_manager_t manager);

#endif