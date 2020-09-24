#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "shutdownManager.h"
#include "sender.h"

#define MSG_MAX_LEN 1024 

static pthread_t threadPID; 
static pthread_mutex_t* s_sendCsMutex;
static List* sendBuf;
static char* remote_name; 
static char* remote_port;

static int sockfd; 
static struct addrinfo *res;

static pthread_cond_t s_syncSendCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_syncSendMutex = PTHREAD_MUTEX_INITIALIZER;

static char* msgItem = NULL; 

void* sendThread(void* args) {
	int status;
        struct addrinfo hints;
	struct addrinfo *p;
	
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if((status = getaddrinfo(remote_name, remote_port, &hints, &res)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		ShutdownManager_trigger();
                exit(1);
        }

	 for(p = res; p != NULL; p = p->ai_next) {
                 if((sockfd = socket(p->ai_family, p->ai_socktype,
                                                p->ai_protocol)) == -1) {
                        perror("sender: socket");
                        continue;
                }

                break;
        }

        while(1) {
		pthread_mutex_lock(&s_syncSendMutex);
                {
                        if(List_count(sendBuf) == 0) {
                                pthread_cond_wait(&s_syncSendCondVar, &s_syncSendMutex);
                        }
                }
		pthread_mutex_unlock(&s_syncSendMutex);

		pthread_mutex_lock(s_sendCsMutex);
                {
                        msgItem = List_trim(sendBuf);
                }
                pthread_mutex_unlock(s_sendCsMutex);

                if(sendto(sockfd, msgItem, MSG_MAX_LEN, 0, res->ai_addr, res->ai_addrlen) == -1) {
                        perror("sendto");
			ShutdownManager_trigger();
                        exit(1);
                }
		
		if(strcmp(msgItem, "!\n") == 0) {
			ShutdownManager_trigger();
			break;
		}

		if(ShutdownManager_isShuttingDown()) {
			break;
		}
	 		
		free(msgItem);
		msgItem = NULL;
        }
	
	return NULL; 
}

void Sender_signalInput()
{
        pthread_mutex_lock(&s_syncSendMutex);
        {
        	pthread_cond_signal(&s_syncSendCondVar);
      	}
	pthread_mutex_unlock(&s_syncSendMutex);
}

void Sender_init(char* their_name, char* their_port, List* buf, 
                 pthread_mutex_t* sendCsMutex) 
{

        s_sendCsMutex = sendCsMutex;
	sendBuf = buf; 
	remote_name = their_name; 
	remote_port = their_port; 

	pthread_create(&threadPID, NULL, sendThread, NULL); 
}

void Sender_shutdown(void) {
	pthread_cancel(threadPID); 
	pthread_join(threadPID, NULL); 
	
	freeaddrinfo(res);
	close(sockfd);

	pthread_cond_destroy(&s_syncSendCondVar);
	pthread_mutex_destroy(&s_syncSendMutex);
	pthread_mutex_destroy(s_sendCsMutex);
}
