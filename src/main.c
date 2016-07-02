
#include "platform.h"

int main(int argc, char** argv){

	// init the process , start another process
	init_daemon();

	// init the config
	config_t t = init_config();



	// destory the config
	config_destory(t);

	return 0;
}