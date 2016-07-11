
#include "platform.h"

int main(int argc, char** argv){

	// init the process , start another process
	int result = init_daemon();
	if (!result){
		ki_log("[ki_dispatcher] : init_daemon is error!\n");
	}

	// init the config
	config_t t = config_init();

	// new the module manager
	module_manager_t manager = module_manager_init(t);

	// init the modules
	module_inits(manager);

	// start all module 
	module_manager_start_all(manager);

	// main thread to sleep one year
	while(1){
		// just sleep some time
		sleep(60*60*24*365);
	}

	// destroy the module manager;
	module_manager_destory(manager);

	// destroy the config
	config_destory(t);

	return 0;
}
