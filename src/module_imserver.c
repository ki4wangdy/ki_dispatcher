
#include "platform.h"

#define status_none		0
#define status_start	1
#define status_over		2

#define imserver_buf_size 512*1024
#define imserver_topic	20;

typedef struct module_imserver_st{
	// lock and cond for another notify 
	pthread_mutex_t lock;
	pthread_cond_t cond;
	// module manager for notify other module
	module_manager_t module_manager;
	// push thread memcacheq fd
	int32_t push_fd;
	// pull thread memcacheq fd
	int32_t pull_fd;
	// push socket
	void* push_socket;
	// pull socket
	void* pull_socket;
	// imserver module status
	int32_t status;
	// pull server flag
	int32_t is_continue;
	// pull buf size
	int8_t pull_buf[imserver_buf_size];
	// push buf size
	int8_t* push_buf;
	// imserver module
	module_t module;
}*module_imserver_t;

static module_imserver_t module_imserver_instance;

static void module_imserver_init(module_manager_t manager){

	module_imserver_instance = r_calloc(1,sizeof(struct module_imserver_st));
	module_imserver_instance->module_manager = manager;

	pthread_mutex_init(&module_imserver_instance->lock,NULL);
	pthread_cond_init(&module_imserver_instance->cond,NULL);

	module_imserver_instance->push_fd = memcacheq_init(manager->config->memcacheq_server,
		manager->config->memcacheq_port);
	ki_log(module_imserver_instance->push_fd <= 0, "[ki_dispatcher] : memcacheq_init in imserver module init procedure failed!\n");

	module_imserver_instance->pull_fd = memcacheq_init(manager->config->memcacheq_server,
		manager->config->memcacheq_port);

	module_imserver_instance->status = status_none;
	module_imserver_instance->is_continue = true;

	module_imserver_instance->module = module_manager_get_module(module_imserver_instance->module_manager, 
		module_flag_imserver);
}

static void module_imserver_pull_process(int8_t* data){
	// just print the data
}

static void module_imserver_push_process(int8_t* data){
	// just print the data
}

static void* pthread_run_push(void* arg){
	module_imserver_t imserver = (module_imserver_t)module_imserver_instance;
	imserver->push_socket = zmq_socket(imserver->module_manager->zmq_context,ZMQ_REQ);
	int sf = zmq_connect(imserver->push_socket,imserver->module_manager->config->imserver_push_ip_addr);
	ki_log(sf != 0, "[ki_dispatcher] : zmq_connect failed in pthread_run_push thread!\n");
	
	char temp[50] = ""; 
	// get the memcache data
	while(imserver->is_continue){
		int len = 0;
#ifdef DEBUG
		fprintf(stderr, "[ki_dispatcher] : memcacheq_get start in pthread_run_push \n");
#endif
		int s = memcacheq_get(imserver->push_fd, imserver->module_manager->config->imserver_ip,
			(char**)&imserver->push_buf,&len);
		// 1 for success 
		if(s == 1){
#ifdef DEBUG
			fprintf(stderr, "[ki_dispatcher] : memcacheq_get success the data:%s in pthread_run_push \n",
				imserver->push_buf);
#endif
			// 1. process data
			imserver->push_buf[len+1] = '\0';
			module_t imserver_module = module_imserver_instance->module;
			if(imserver_module != NULL){
				imserver_module->module_push_process(imserver->push_buf);
			}
			// 2. push the data to zmq
			int fs = zmq_send(imserver->push_socket,imserver->push_buf,len,0);
			r_free(imserver->push_buf);
			imserver->push_buf = NULL;
			if(fs != len){
				ki_log(fs != len, "[ki_dispatcher] : zmq_send failed in pthread_run_push!\n");
			}
			int sf = zmq_recv(imserver->push_socket, imserver->pull_buf, imserver_buf_size, 0);
			if(sf <= 0){
				ki_log(sf <= 0, "[ki_dispatcher] : zmq_recv failed in pthread_run_push!\n");
			}
		} 
		// s for nothing , so wait
		else if(s == 0){
#ifdef DEBUG
			fprintf(stderr, "[ki_dispatcher] : memcacheq_get nothing and will wait in pthread_run_push \n");
#endif
			pthread_mutex_lock(&imserver->lock);
			pthread_cond_wait(&imserver->cond,&imserver->lock);
			pthread_mutex_unlock(&imserver->lock);
		} 
		// error to assert
		else {
			ki_log(true, "memcacheq_get data is error in pthread_run_push !\n");
		}
	}

	pthread_detach(pthread_self());
	return NULL;
}

static void* pthread_run_pull(void* arg){

	module_imserver_t imserver = (module_imserver_t)module_imserver_instance;
	imserver->pull_socket = zmq_socket(imserver->module_manager->zmq_context,ZMQ_REP);
	int sf = zmq_connect(imserver->pull_socket,imserver->module_manager->config->imserver_pull_ip_addr);
	ki_log(sf != 0, "[ki_dispatcher] : zmq_connect failed in pthread_run_pull thread!\n");

	int32_t s = 0;
	char buf[] = "ack";
	while(imserver->is_continue){
		s = zmq_recv(imserver->pull_socket,imserver->pull_buf,imserver_buf_size,0);
		zmq_send(imserver->pull_socket,buf,strlen(buf),0);
		if(s >= imserver_buf_size){
			ki_log(s >= imserver_buf_size, "zmq_recv's buf is so big in pthread_run_pull! \n");
		}
		if(s > 0){
			// 1.pull the data and process
			imserver->pull_buf[s+1] = '\0';
			module_t imserver_module = module_imserver_instance->module;
			if(imserver_module != NULL){
				imserver_module->module_pull_process(imserver->pull_buf);
			}
			// 2. push the data to memcache cache
			int sf = memcacheq_set(imserver->pull_fd,imserver->module_manager->config->schat_topic,
				imserver->pull_buf,s);
			if(sf <= 0){
				ki_log(sf <= 0, "[ki_dispatcher] : memcacheq set failed sf <= 0 in pthread_run_pull!\n");
			}
			// 3.notify other module to get data from memcache queue
			module_manager_notify(imserver->module_manager,module_flag_single_chat);
			continue;
		}
		ki_log(s >= imserver_buf_size, "zmq pull's size is <= 0 in pthread_run_pull! \n");
	}

	pthread_detach(pthread_self());
	return NULL;
}

static void module_imserver_start(){

	int s = 0;
	pthread_t pull_pthread;
	s = pthread_create(&pull_pthread,NULL,pthread_run_pull,NULL);
	ki_log(s != 0, "[ki_dispatcher] : module_imserver_start failed because of pthread_run_pull pthread \n");

	pthread_t push_pthread;
	s = pthread_create(&push_pthread,NULL,pthread_run_push,NULL);
	ki_log(s != 0, "[ki_dispatcher] : module_imserver_start failed because of pthread_run_push pthread \n");

}

static void module_imserver_notify(){
	module_imserver_t imserver = module_imserver_instance ;
	pthread_mutex_lock(&imserver->lock);
	pthread_cond_broadcast(&imserver->cond);
	pthread_mutex_unlock(&imserver->lock);
}

static void module_imserver_destory(){
	assert(module_imserver_instance != NULL);
	module_imserver_instance->module_manager = NULL;

	// destory lock and cond
	pthread_mutex_destroy(&module_imserver_instance->lock);
	pthread_cond_destroy(&module_imserver_instance->cond);

	// close the memcache queue
	memcacheq_close(module_imserver_instance->push_fd);
	memcacheq_close(module_imserver_instance->pull_fd);

	// close push and pull socket
	zmq_close(module_imserver_instance->push_socket);
	zmq_close(module_imserver_instance->pull_socket);

	r_free(module_imserver_instance);
	module_imserver_instance = NULL;
}

module_t module_imserver_inits(module_manager_t manager){
	static struct module_st imserver_module = {
		module_imserver_init,
		module_imserver_start,
		module_imserver_notify,
		module_imserver_pull_process,
		module_imserver_push_process,
		module_imserver_destory
	};
	return &imserver_module;
}
