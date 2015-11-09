#ifndef __REQUEST_H__

void requestHandle(int fd);

enum ERROR_CODE{SUCCESS, ERROR};
typedef struct _request_info_
{
  int is_static;               // denote whether the request is static or dynamic
  char filename[MAXLINE];      // name of the requested file
  int connfd;                  // connect file descriptor
  enum ERROR_CODE error_code;  // error code: 0-SUCCESS, 1-ERROR
} __request_info_;

typedef struct _buffer_
{
	int buffer_size;           // size of the buffer
	int head;                  // head index 
	int length;                // length of the queue
	__request_info_ * request;
} __buffer_;


int isBufEmpty(volatile __buffer_ * buf){
	//return (buf->tail == buf->head)?1:0;
	return buf->length == 0;
}

int isBufFull(volatile __buffer_ * buf){
	//return ((buf->tail+1)%buf->buffer_size == buf->head)?1:0;
	return buf->length == buf->buffer_size;
}

__request_info_ deBuf(volatile __buffer_ * buf){
	__request_info_ temp = buf->request[buf->head];
	buf->head = (buf->head+1)%buf->buffer_size;
	buf->length --;
	return temp;
}

void enBuf(volatile __buffer_ * buf, __request_info_ req){
	//buf->tail = (buf->tail+1)%buf->buffer_size;
	buf->request[buf->head+buf->length] = req;
	buf->length ++;
}


volatile __buffer_ myBuffer;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

/* consumers */
void * worker(void *arg) {
	while(1){
		printf("worker\n");
		pthread_mutex_lock(&mutex);
		while(isBufEmpty(&myBuffer)){
			pthread_cond_wait(&empty, &mutex);
		}
		
		__request_info_ req = deBuf(&myBuffer);
		printf("connfd = %d\n", req.connfd);
		requestHandle(req.connfd);
	
		Close(req.connfd);

		pthread_cond_signal(&full);
		pthread_mutex_unlock(&mutex);

	}
	return NULL;
}



#endif
