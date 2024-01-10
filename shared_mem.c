
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

#include "structs.h"

int createSHM(key_t key, int size){
    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if(shmid == -1){
        perror("shmget");
        exit(1);
    }
    return shmid;
}

void *accessSHM(int shmid){
    void *shm = shmat(shmid, (void *)0, 0);
    if(shm == (void *)-1){
        perror("shmat");
        exit(1);
    }
    return shm;
}

void detachSHM(void *shm){
    if(shmdt(shm) == -1){
        perror("shmdt");
        exit(1);
    }
}

void deleteSHM(int shmid){
    if(shmctl(shmid, IPC_RMID, 0) == -1){
        perror("shmctl");
        exit(1);
    }
}
