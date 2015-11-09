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



#endif
