#ifndef _SHUTDOWNMANAGER_H_
#define _SHUTDOWNMANAGER_H_

void ShutdownManager_wait();
void ShutdownManager_trigger();
int ShutdownManager_isShuttingDown(); 

#endif
