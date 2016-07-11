
#include "platform.h"

int main(int argc, char** argv){

	// init the process , start another process
	int result = init_daemon();
	ki_log(!result, "[ki_dispatcher] : init_daemon is error!\n");

	// init the config
	config_t t = config_init();
	ki_print("[ki_dispatcher] : init config over !\n");

	// init the modules
	module_manager_t manager = module_manager_init(t);
	module_inits(manager);
	ki_print("[ki_dispatcher] : init thread over !\n");

	// start all module 
	module_manager_start_all(manager);

	// main thread to sleep one year
	while(1){
		ki_print("[ki_dispatcher] : main pthread start to sleep !\n");
		// just sleep some time
		sleep(60*60*24*365);
	}

	// destroy the module manager;
	module_manager_destory(manager);

	// destroy the config
	config_destory(t);

	return 0;
}
