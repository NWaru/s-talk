#ifndef _WRITER_H
#define _WRITER_H

#include <pthread.h>
#include "list.h" 

void Writer_signalOutput();
void Writer_init(List* buf, pthread_mutex_t* recvCsMutex); 
void Writer_shutdown(); 

#endif
