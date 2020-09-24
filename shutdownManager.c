#include <pthread.h>
#include "shutdownManager.h"

static pthread_cond_t s_shutdownCondVar = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_shutdownMutex = PTHREAD_MUTEX_INITIALIZER; 
static int s_shutdownFlag = 0;

void ShutdownManager_wait() 
{
	
	pthread_mutex_lock(&s_shutdownMutex);
	{
		pthread_cond_wait(&s_shutdownCondVar, &s_shutdownMutex);
	}
	pthread_mutex_unlock(&s_shutdownMutex);
}

void ShutdownManager_trigger() 
{
	s_shutdownFlag = 1;		
	pthread_mutex_lock(&s_shutdownMutex);
	{
		pthread_cond_signal(&s_shutdownCondVar);
	}
	pthread_mutex_unlock(&s_shutdownMutex);
	

	pthread_cond_destroy(&s_shutdownCondVar);
	pthread_mutex_destroy(&s_shutdownMutex); 
}

int ShutdownManager_isShuttingDown() 
{
	return s_shutdownFlag;
}

