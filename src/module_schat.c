
#include "platform.h"

#define status_none		0
#define status_start	1
#define status_over		2

#define schat_buf_size 512*1024
#define schat_topics	20

typedef struct module_schat_st{
	// lock and cond for another notify 
	pthread_mutex_t lock;
	pthread_cond_t cond;
	// module manager for notify other module
	module_manager_t module_manager;
	// cache
	int32_t fd;
	// push socket
	void* push_socket;
	// pull socket
	void* pull_socket;
	// imserver module status
	int32_t status;
	// pull server flag
	int32_t is_continue;
	// pull buf size
	int8_t pull_buf[schat_buf_size];
	// push buf size
	int8_t* push_buf;
	// schat module
	module_t module;
}*module_schat_t;

static module_schat_t module_schat_instance;

static void module_schat_init(module_manager_t manager){
	module_schat_instance = r_calloc(1,sizeof(struct module_schat_st));
	module_schat_instance->module_manager = manager;

	pthread_mutex_init(&module_schat_instance->lock,NULL);
	pthread_cond_init(&module_schat_instance->cond,NULL);

	module_schat_instance->fd = memcacheq_init(manager->config->memcacheq_server,
		manager->config->memcacheq_port);
	assert(module_schat_instance->fd > 0);

	module_schat_instance->status = status_none;
	module_schat_instance->is_continue = true;

	module_schat_instance->module = module_manager_get_module(module_schat_instance->module_manager, 
		module_flag_single_chat);

}

static void* pthread_run_push(void* arg){

	module_schat_t imserver = (module_schat_t)module_schat_instance;
	imserver->push_socket = zmq_socket(imserver->module_manager->zmq_context,ZMQ_REQ);
	zmq_connect(imserver->push_socket,imserver->module_manager->config->schat_push_ip_addr);

	char temp[50] = ""; 
	// get the memcache data
	while(imserver->is_continue){
		int len = 0;
		int s = memcacheq_get(imserver->fd,imserver->module_manager->config->schat_topic,
			(char**)&imserver->push_buf,&len);
		// 1 for success 
		if(s == 1){
			// 1. process data
			imserver->push_buf[len+1] = '\0';
			module_t imserver_module = module_schat_instance->module;
			if(imserver_module != NULL){
				imserver_module->module_push_process(imserver->push_buf);
			}
			// 2. push the data to zmq
			int fs = zmq_send(imserver->push_socket,imserver->push_buf,len,0);
			if(fs != len){
				fprintf(stderr,"pthread_run_push push data is error!\n");
				assert(0);
			}
			int sf = zmq_recv(imserver->push_socket, imserver->pull_buf, schat_buf_size, 0);
			if(sf <= 0){
				fprintf(stdout,"pthread_run_push pull data is error!\n");
				assert(0);
			}
			// 3. get the data and push to memcacheq 
			imserver->pull_buf[sf + 1] = '\0';
			if (imserver_module != NULL){
				imserver_module->module_pull_process(imserver->pull_buf);
			}
			sf = memcacheq_set(imserver->fd, imserver->module_manager->config->imserver_ip,
				imserver->pull_buf, s);
			if (sf <= 0){
				fprintf(stderr, "pthread_run_pull memcacheq set error!\n");
				assert(0);
			}
			// 4.notify other module to get data from memcache queue
			module_manager_notify(imserver->module_manager, module_flag_imserver);
		} 
		// s for nothing , so wait
		else if(s == 0){
			pthread_mutex_lock(&imserver->lock);
			pthread_cond_wait(&imserver->cond,&imserver->lock);
			pthread_mutex_unlock(&imserver->lock);
		} 
		// error to assert
		else {
			fprintf(stderr,"memcacheq pull data is error !\n");
			assert(0);
		}
	}

	pthread_detach(pthread_self());
	return NULL;
}

static void module_schat_start(){
	int s = 0;
	pthread_t push_pthread;
	s = pthread_create(&push_pthread,NULL,pthread_run_push,NULL);
	assert(s == 0);
}

static void module_schat_notify(){
	module_schat_t schatServer = module_schat_instance ;
	pthread_mutex_lock(&schatServer->lock);
	pthread_cond_broadcast(&schatServer->cond);
	pthread_mutex_unlock(&schatServer->lock);
}

static void module_schat_push_process(int8_t* data){
	// just print the data
}

static void module_schat_pull_process(int8_t* data){
	// just print the data
}

static void module_schat_destory(){
	assert(module_schat_instance != NULL);
	module_schat_instance->module_manager = NULL;

	// destory lock and cond
	pthread_mutex_destroy(&module_schat_instance->lock);
	pthread_cond_destroy(&module_schat_instance->cond);

	// close the memcache queue
	memcacheq_close(module_schat_instance->fd);

	// close push and pull socket
	zmq_close(module_schat_instance->push_socket);
	zmq_close(module_schat_instance->pull_socket);

	r_free(module_schat_instance);
	module_schat_instance = NULL;
}

module_t module_schat_inits(module_manager_t manager){
	static struct module_st module_instance = {
		module_schat_init,
		module_schat_start,
		module_schat_notify,
		module_schat_pull_process,
		module_schat_push_process,
		module_schat_destory
	};	
	return &module_instance;

}

