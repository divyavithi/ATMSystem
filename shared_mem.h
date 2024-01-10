
#ifndef SHARED_MEM_H
#define SHARED_MEM_H
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

int createSHM(key_t key, int size);
void *accessSHM(int shmid);
void detachSHM(void *shm);
void deleteSHM(int shmid);

#endif