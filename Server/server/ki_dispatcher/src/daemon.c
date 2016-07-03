
#include "platform.h"

int init_daemon(){

	int32_t fd;

	switch (fork()){
		case ki_error:
			return false;
		case ki_ok:
			break;
		default:
			exit(0);
			break;
	}

	if (setsid() == -1){
		return false;
	}

	umask(0);

	fd = open("/dev/null", O_RDWR);
	if (fd == ki_error){
		return false;
	}

	if (dup2(fd, STDIN_FILENO) == -1) {
		return false;
	}

	if (dup2(fd, STDOUT_FILENO) == -1) {
		return false;
	}

// 	if (fd > STDERR_FILENO) {
// 		if (close(fd) == -1) {
// 			return false;
// 		}
// 	}
	return true;
}

