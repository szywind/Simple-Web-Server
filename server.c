#include "cs537.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//


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
	buf->request[(buf->head+buf->length)%buf->buffer_size] = req;
	buf->length ++;
}


volatile __buffer_ myBuffer;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

/* consumers */
void * worker(void *arg) {
	while(1){
		//printf("worker\n");
		pthread_mutex_lock(&mutex);
		while(isBufEmpty(&myBuffer)){
			pthread_cond_wait(&empty, &mutex);
		}
		
		__request_info_ req = deBuf(&myBuffer);
		pthread_cond_signal(&full);
		pthread_mutex_unlock(&mutex);
		//printf("connfd = %d\n", req.connfd);
		requestHandle(req.connfd);
	
		Close(req.connfd);



	}
	return NULL;
}



// CS537: Parse the new arguments too
void getargs(int *args, int argc, char *argv[])
{
    if (argc != 4) {
		fprintf(stderr, "Usage: %s <port> <threads> <buffers>\n", argv[0]);
		exit(1);
    }
    args[0] = atoi(argv[1]);
    args[1] = atoi(argv[2]);
    args[2] = atoi(argv[3]);	
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, args[3], clientlen;
    struct sockaddr_in clientaddr;

    getargs(args, argc, argv); // get the port number specified by users
    int port = args[0];
    int nThreads = args[1];
    int szBuffer = args[2];
    // 
    // CS537: Create some threads...
   
    // initialize the buffer
    myBuffer.buffer_size = szBuffer;
    myBuffer.head = 0;
    myBuffer.length = 0;
    myBuffer.request = NULL;
    if(NULL == (myBuffer.request = (__request_info_ *)malloc(szBuffer*sizeof(__request_info_)))){
  		perror("create buffer error!\n");
		exit(1);  	
    }

	// create thread pool
	pthread_t * pool = NULL;
	if(NULL == (pool = (pthread_t *) malloc(nThreads * sizeof(pthread_t)))){
		perror("create pool error!\n");
		exit(1);
	}

    // create workers
    unsigned int i = 0;
    for(; i<nThreads; i++){
    	pthread_create(&pool[i], NULL, worker, NULL);
    }

    listenfd = Open_listenfd(port); // listening socket file descriptor
    while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

		// 
		// CS537: In general, don't handle the request in the main thread.
		// Save the relevant info in a buffer and have one of the worker threads 
		// do the work. However, for SFF, you may have to do a little work
		// here (e.g., a stat() on the filename) ...
		// 

	    /* producer */
	    pthread_mutex_lock(&mutex);
		while(isBufFull(&myBuffer)){
			pthread_cond_wait(&full, &mutex);
		}

		__request_info_ req;
		req.connfd = connfd;

		//printf("connfd = %d\n", connfd);

		enBuf(&myBuffer, req);

		//printf("sz = %d, head = %d, length = %d, request->connfd = %d\n", myBuffer.buffer_size, myBuffer.head, myBuffer.length, myBuffer.request[0].connfd);
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);		

		//requestHandle(connfd);

		//Close(connfd);
    }
    // free the pool and buffer
    free(pool);
    free(myBuffer.request);
}


    


 
