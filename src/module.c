
#include "platform.h"

void module_inits(module_manager_t manager){

	// 1. register the im server module
	module_t imserver = module_imserver_inits(manager);
	module_manager_add_module(manager,module_flag_imserver,imserver);

	// 2. register the schat module 
	module_t schat = module_schat_inits(manager);
	module_manager_add_module(manager,module_flag_single_chat,schat);

}
