
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <sys/sem.h>

int waitSemaphore(int sem_id);
int signalSemaphore(int sem_id);
void removeSemaphore(int sem_id);
int createSemaphore(int initialVal);

#endif