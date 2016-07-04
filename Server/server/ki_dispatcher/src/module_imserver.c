
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

	module_imserver_instance->fd = memcacheq_init(manager->config->memcacheq_server,
		manager->config->memcacheq_port);
	assert(module_imserver_instance->fd > 0);

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
	zmq_connect(imserver->push_socket,imserver->module_manager->config->imserver_push_ip_addr);
	
	char temp[50] = ""; 
	// get the memcache data
	while(imserver->is_continue){
		int len = 0;
		int s = memcacheq_get(imserver->fd,imserver->module_manager->config->imserver_ip,
			(char**)&imserver->push_buf,&len);
		// 1 for success 
		if(s == 1){
			// 1. process data
			imserver->push_buf[s+1] = '\0';
			module_t imserver_module = module_imserver_instance->module;
			if(imserver_module != NULL){
				imserver_module->module_push_process(imserver->push_buf);
			}
			// 2. push the data to zmq
			int fs = zmq_send(imserver->push_socket,imserver->push_buf,len,0);
			r_free(imserver->push_buf);
			imserver->push_buf = NULL;
			if(fs != len){
				fprintf(stderr,"pthread_run_push push data is error!\n");
				assert(0);
			}
			int sf = zmq_recv(imserver->push_socket, imserver->pull_buf, imserver_buf_size, 0);
			if(sf <= 0){
				fprintf(stdout,"pthread_run_push pull data is error!\n");
				assert(0);
			}
			// 4.notify other module to get data from memcache queue
			module_manager_notify(imserver->module_manager, module_flag_single_chat);
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

static void* pthread_run_pull(void* arg){

	module_imserver_t imserver = (module_imserver_t)module_imserver_instance;
	imserver->pull_socket = zmq_socket(imserver->module_manager->zmq_context,ZMQ_REP);
	zmq_connect(imserver->pull_socket,imserver->module_manager->config->imserver_pull_ip_addr);

	int32_t s = 0;
	char buf[] = "ack";
	while(imserver->is_continue){
		s = zmq_recv(imserver->pull_socket,imserver->pull_buf,imserver_buf_size,0);
		zmq_send(imserver->pull_socket,buf,strlen(buf),0);
		if(s >= imserver_buf_size){
			fprintf(stderr,"error to pull, the size is so big ! \n");
			assert(0);
		}
		if(s > 0){
			// 1.pull the data and process
			imserver->pull_buf[s+1] = '\0';
			module_t imserver_module = module_imserver_instance->module;
			if(imserver_module != NULL){
				imserver_module->module_pull_process(imserver->pull_buf);
			}
			// 2. push the data to memcache cache
			int sf = memcacheq_set(imserver->fd,imserver->module_manager->config->schat_topic,
				imserver->pull_buf,s);
			if(sf <= 0){
				fprintf(stderr,"pthread_run_pull memcacheq set error!\n");
				assert(0);
			}
			// 3.notify other module to get data from memcache queue
			module_manager_notify(imserver->module_manager,module_flag_single_chat);
			continue;
		}
		fprintf(stderr,"zmq pull's size is <= 0 ! \n");
	}

	pthread_detach(pthread_self());
	return NULL;
}

static void module_imserver_start(){

	int s = 0;
	pthread_t pull_pthread;
	s = pthread_create(&pull_pthread,NULL,pthread_run_pull,NULL);
	assert(s == 0);

	pthread_t push_pthread;
	s = pthread_create(&push_pthread,NULL,pthread_run_push,NULL);
	assert(s == 0);

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
	memcacheq_close(module_imserver_instance->fd);

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
