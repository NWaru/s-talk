#ifndef _READER_H
#define _READER_H

#include <pthread.h>
#include "list.h"

void Reader_init(List* buf, pthread_mutex_t* sendCsMutex);
void Reader_shutdown(void);

#endif
