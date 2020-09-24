#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "writer.h" 
#include "shutdownManager.h"

#define SCFD 1
#define MSG_MAX_LEN 1024

static pthread_t threadPID; 
static pthread_mutex_t* s_recvCsMutex;
static List* recvBuf; 

static pthread_cond_t s_syncRecvCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncRecvMutex = PTHREAD_MUTEX_INITIALIZER;

static char* msgItem = NULL; 

void* writeThread(void* args) 
{
	while(1) {
		pthread_mutex_lock(&s_syncRecvMutex);
		{
			if(List_count(recvBuf) == 0) {
				pthread_cond_wait(&s_syncRecvCondVar, &s_syncRecvMutex); 
			}
		}
		pthread_mutex_unlock(&s_syncRecvMutex); 

		pthread_mutex_lock(s_recvCsMutex); 
		{
			msgItem = List_trim(recvBuf);
		}
		pthread_mutex_unlock(s_recvCsMutex); 

		if(msgItem != NULL)
		{
			if(fputs(msgItem, stdout) == -1) {
				fprintf(stderr, "writer: fputs");
				ShutdownManager_trigger();
				exit(1);
			}
		}

		if(strcmp(msgItem, "!\n") == 0) {
			ShutdownManager_trigger();
			break;
		}

		if(ShutdownManager_isShuttingDown()) {
			break;
		}

		if(msgItem != NULL) {
			free(msgItem);
			msgItem = NULL;
		}
	}	
	return NULL; 
}

void Writer_signalOutput() 
{
	pthread_mutex_lock(&s_syncRecvMutex);
        {
        	pthread_cond_signal(&s_syncRecvCondVar);
        }
      	pthread_mutex_unlock(&s_syncRecvMutex);
}

void Writer_init(List* buf, pthread_mutex_t* recvCsMutex)
{
	s_recvCsMutex = recvCsMutex;	
	recvBuf = buf; 

	pthread_create(&threadPID, NULL, writeThread, NULL);  
}

void Writer_shutdown(void) 
{
        pthread_cancel(threadPID);
        pthread_join(threadPID, NULL);
/*
	if(msgItem != NULL) {
		free(msgItem); 
		msgItem = NULL; 
	}
	*/
	
	pthread_cond_destroy(&s_syncRecvCondVar);
	pthread_mutex_destroy(&s_syncRecvMutex);
	pthread_mutex_destroy(s_recvCsMutex);
}

