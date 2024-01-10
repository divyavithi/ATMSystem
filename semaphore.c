#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>

#include "semaphore.h"
// to pass arguments to semctl
union semun{
    int val;
    struct semid_ds *buf;
    ushort *array;
};

struct sembuf sbOperation;

int waitSemaphore(int sem_id){
    sbOperation.sem_num = 0;
    sbOperation.sem_op = -1;
    sbOperation.sem_flg = SEM_UNDO;

    return semop(sem_id, &sbOperation, 1);
}

int signalSemaphore(int sem_id){
    sbOperation.sem_num = 0;
    sbOperation.sem_op = +1;
    sbOperation.sem_flg = 0;

    return semop(sem_id, &sbOperation, 1);
}

void removeSemaphore(int sem_id){
    if(sem_id != -1) { //IPC error
        semctl(sem_id, 0, IPC_RMID, 0);
    }
}

int createSemaphore(int initialVal){
    int sem_id;
    union semun suInitData;

    // get semaphore
    sem_id = semget(IPC_PRIVATE, 1, 0666);

    // checking for errors
    if (sem_id == -1){
        return sem_id;
    }

    // initialize the semaphore
    suInitData.val = initialVal;
    if(semctl(sem_id, 0, SETVAL, suInitData) == -1){
        // error occurred, so remove semaphore
        removeSemaphore(sem_id);
        return -1;
    }
    return sem_id;
}