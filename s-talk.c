#include <stdio.h>
#include <stdlib.h>

#include "receiver.h"
#include "writer.h" 
#include "sender.h"
#include "reader.h" 



#include "list.h"
#include "shutdownManager.h"

int main(int argc, char* argv[]) {
	if(argc != 4) {
                fprintf(stderr, "usage: s-talk [my port number]" 
				"[remote machine name] [remote port number]\n");

                exit(1);
        }

	pthread_mutex_t s_recvCsMutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_t s_sendCsMutex = PTHREAD_MUTEX_INITIALIZER;

	List* sendBuf = List_create(); 
	List* recvBuf = List_create(); 
	
	Receiver_init(argv[1], recvBuf, &s_recvCsMutex); 
	Writer_init(recvBuf, &s_recvCsMutex);
	Reader_init(sendBuf, &s_sendCsMutex);
	Sender_init(argv[2], argv[3], sendBuf, &s_sendCsMutex); 

	ShutdownManager_wait(); 

	Receiver_shutdown();
	Writer_shutdown();
	Sender_shutdown(); 
	Reader_shutdown();

	List_free(sendBuf, NULL); 
	List_free(recvBuf, NULL);
	
	return 0; 
}
