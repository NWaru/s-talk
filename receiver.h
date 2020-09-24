#ifndef _RECEIVER_H_
#define _RECEIVER_H_

#include <pthread.h>
#include "list.h"

void Receiver_init(char* port, List* buf, pthread_mutex_t* recvCsMutex);
void Receiver_shutdown(void); 

#endif
