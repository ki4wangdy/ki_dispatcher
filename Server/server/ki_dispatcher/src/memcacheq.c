
#include "platform.h"
#include "memcacheq.h"

//static pthread_mutex_t lock;

int memcacheq_init(char* server, int port){

//	pthread_mutex_init(&lock, NULL);

	int sockfd;
	struct sockaddr_in server_addr;
	struct hostent *host;
	int portnumber;
	
	host = gethostbyname(server);
	portnumber = port;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		ki_log(sockfd == -1, "[ki_dispatcher] : memcacheq init socket failed! %s\a\n", strerror(errno));
		return 0;
	}
	
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnumber);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	
	if (connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1){
		ki_log(true, "[ki_dispatcher] : memcacheq init connect failed! %s\a\n", strerror(errno));
		return 0;
	 }
	return sockfd;
}

// if the return value is true, the set handle is success
int memcacheq_set(int fd, char* topic, char* value, int value_len){

//	pthread_mutex_lock(&lock);

	int st = -1;
	int nbytes = 0;
	if (fd == 0){
		st = -1;
		ki_log(fd == 0, "[ki_dispatcher] : memcacheq_set fd == 0! \n");
		goto end;
	}

	char buf[1024*10] = "";
	snprintf(buf, 1024*10, "set %s 0 0 %d\r\n%s\r\n", topic, value_len, value);

	int len = strlen(buf);
	int s = write(fd, buf, len);
	if (s <= 0){
		st = -1;
		ki_log(s <= 0, "[ki_dispatcher] : memcacheq_set write failed!\n");
		goto end;
	}

	if ((nbytes = read(fd, buf, 100)) == -1){
		ki_log(s <= 0, "[ki_dispatcher] : memcacheq_set read failed! %s\n",strerror(errno));
		st = -1;
		goto end;
	}

	buf[nbytes] = '\0';
	if (strstr(buf,"STORED") != 0){
		st = 1;
	}

end:
//	pthread_mutex_unlock(&lock);
	return st ;
}

// if the return value is false, then the topic queue has no message
int memcacheq_get(int fd, char* topic, char** value, int* len){

//	pthread_mutex_lock(&lock);

	int st = 1;
	int nbytes = 0;
	if (fd == 0){
		st = -1;
		ki_log(fd == 0, "[ki_dispatcher] : memcacheq_get fd == 0! \n");
		goto end;
	}

	char buf[1024*512] = "";
	snprintf(buf, 1024 * 512, "get %s\r\n", topic);

	int temp_len = strlen(buf);
	int s = write(fd, buf, temp_len);
	if (s <= 0){
		st = -1;
		goto end;
	}
	assert(s == temp_len);
	if ((nbytes = read(fd, buf, 1024 * 512)) == -1){
		ki_log(s <= 0, "[ki_dispatcher] : memcacheq_get read failed!\n");
		st = -1;
		goto end;
	}
	buf[nbytes] = '\0';
	char r[] = "END\r\n";
	if (memcmp(buf,r,sizeof(r)-1) == 0){
		st = 0;
		goto end;
	}

	char prefix[100] = "";
	snprintf(prefix, 100, "VALUE %s 0 ", topic);
	int prefix_size = strlen(prefix);
	if (nbytes <= prefix_size){
		st = -1;
		goto end;
	}

	int i = prefix_size + 1;
	int j = 1;
	for (; i < nbytes; i++,j++){
		if (buf[i] == '\r'){
			break;
		}
	}

	char dital_string[10] = "";
	const char* temp = &buf[prefix_size];
	memcpy(dital_string, temp, j);
	int size = atoi(dital_string);

	assert(size > 0);
	*len = size;

	const char* temps = &buf[prefix_size + j + sizeof("\r\n") - 1];
	char* result = calloc(1, size);
	memcpy(result, temps, size);
	*value = result;

end:
//	pthread_mutex_unlock(&lock);
	return st;

}

int memcacheq_close(int fd){
	if (fd <= 0){
		return 0;
	}
	close(fd);
//	pthread_mutex_destroy(&lock);
}

