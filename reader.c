#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "sender.h"
#include "shutdownManager.h"

#define MSG_MAX_LEN 1024

static pthread_t threadPID; 
static pthread_mutex_t* s_sendCsMutex;
static List* sendBuf;
static char* msgItem; 

void* readThread(void* args) 
{
	while(1) {
		if((msgItem = (char*)calloc(MSG_MAX_LEN, sizeof(char))) == NULL) {
			fprintf(stderr, "reader: failed to allocate memory\n");
			ShutdownManager_trigger();
                        exit(1);
		}

		if(fgets(msgItem, MSG_MAX_LEN, stdin) == NULL) {
			fprintf(stderr, "reader: failed to read to buffer\n");			     
			ShutdownManager_trigger();
                	exit(1);
		}
		
		pthread_mutex_lock(s_sendCsMutex);
                {
                        if(List_prepend(sendBuf, msgItem) == -1) {
				fprintf(stderr, "reader: internal data structure full\n");
                       		ShutdownManager_trigger();
				exit(1); 
			}
                }
                pthread_mutex_unlock(s_sendCsMutex);

		Sender_signalInput(); 

		if(strcmp(msgItem, "!\n") == 0 || ShutdownManager_isShuttingDown()) {
			break;
		}
		
	}
		
	return NULL;
}

void Reader_init(List* buf, pthread_mutex_t* sendCsMutex)
{
        s_sendCsMutex = sendCsMutex;
        sendBuf = buf;

	pthread_create(&threadPID, NULL, readThread, NULL); 
}

void Reader_shutdown(void)
{
	pthread_cancel(threadPID);
        pthread_join(threadPID, NULL);

	if(msgItem != NULL) {
		free(msgItem); 
		msgItem = NULL;
	}

        pthread_mutex_destroy(s_sendCsMutex);
}

