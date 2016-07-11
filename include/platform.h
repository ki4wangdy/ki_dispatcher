
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>    
#include <sys/stat.h>    

#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <errno.h>

#include <netdb.h>
#include <netinet/in.h>

#include <pthread.h>
#include <zmq.h>

#include "type.h"
#include "platform.h"
#include "daemon.h"
#include "chash.h"
#include "memcacheq.h"
#include "config.h"
#include "log.h"

#include "module.h"
#include "module_manager.h"

#include "module_imserver.h"
#include "module_schat.h"

#endif