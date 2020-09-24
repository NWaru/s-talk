#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "receiver.h"
#include "writer.h"
#include "shutdownManager.h"

#define MSG_MAX_LEN 1024

static pthread_t threadPID;
static pthread_mutex_t* s_recvCsMutex;
static char* my_port;
static List* recvBuf; 

int yes = 1;
static int sockfd;
static struct addrinfo *res;

static char* msgItem; 

void* receiveThread(void* args) 
{
	int status;
        struct addrinfo hints;
	struct addrinfo *p;
        struct sockaddr_storage remote_addr;
        socklen_t addr_len = 0;

	memset(&remote_addr, 0, sizeof(remote_addr));
        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        if((status = getaddrinfo(NULL, my_port, &hints, &res)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		ShutdownManager_trigger();
                exit(1);
        }

        for(p = res; p != NULL; p = p->ai_next) {
		 if((sockfd = socket(p->ai_family, p->ai_socktype,
                                                p->ai_protocol)) == -1) {
                        perror("receiver: socket");
                        continue;
                }

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
				     &yes, sizeof(int)) == -1) {
			perror("receiver: setsockopt");
			ShutdownManager_trigger();
			exit(1);
		}	

                if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                        perror("receiver: bind");
                        continue;
                }

                break;
        }

        if(p == NULL) {
                fprintf(stderr, "receiver: failed to bind\n");
		ShutdownManager_trigger();
                exit(1);
        }

        while(1) {
		if((msgItem = (char*)calloc(MSG_MAX_LEN, sizeof(char))) == NULL) {
			fprintf(stderr, "receiver: failed to allocate memory\n");
			ShutdownManager_trigger(); 
			exit(1);
		}
	
                if((recvfrom(sockfd, msgItem, MSG_MAX_LEN, 
					0, (struct sockaddr*)&remote_addr, &addr_len)) == -1) 
		{
                        perror("recvfrom");
			ShutdownManager_trigger();
                        exit(1);
                }
		
		pthread_mutex_lock(s_recvCsMutex);
                {
			if(List_prepend(recvBuf, msgItem) == -1) {
				fprintf(stderr, "receiver: internal data structure full\n");
				ShutdownManager_trigger();
				exit(1);
			}
                }
                pthread_mutex_unlock(s_recvCsMutex);

		Writer_signalOutput();

		if(strcmp(msgItem, "!\n") == 0 || ShutdownManager_isShuttingDown()) {
			break; 
		}
        }

	return NULL;
}

void Receiver_init(char* port, List* buf, pthread_mutex_t* recvCsMutex)
{
	s_recvCsMutex = recvCsMutex;
	my_port = port;
	recvBuf = buf; 	

	pthread_create(&threadPID, NULL, receiveThread, NULL); 	
}

void Receiver_shutdown(void) 
{
	pthread_cancel(threadPID);
	pthread_join(threadPID, NULL); 

	if(msgItem != NULL) {
		free(msgItem); 
		msgItem = NULL;
	}
	
	freeaddrinfo(res);
	close(sockfd);

        pthread_mutex_destroy(s_recvCsMutex);

}
