#ifndef _SENDER_H_
#define _SENDER_H_ 

#include <pthread.h>
#include "list.h" 

void Sender_signalInput();
void Sender_init(char* their_name, char* their_port, List* buf, pthread_mutex_t* sendCsMutex); 
void Sender_shutdown(void);

#endif
