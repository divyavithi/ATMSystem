/**
 * Assignment 3 - Concurrency, IPC,
 * Semaphores, Shared and Virtual Memory, Files
 *
 * Part B - DB Editor Solution
 *
 * Created by Divya Vithiyatharan and Claire Villanueva
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

#include "semaphore.h"
#include "structs.h"
#include "msg_queue.h"
#include "shared_mem.h"

#define SHM_KEY 1234
#define MSG_KEY 5678
#define SEM_KEY 2023

void run_editor(pid_t server_pid){
    bool loop = true;
    bool valid_inputs = true;
    int shm_id, sem_id, msgq_id;
    message_t send_message;
    send_message.msg_type = 4; // always assigned to 4 for UPDATE_DB (b/c editor only has one operation)

    account_t temp_acc;
    account_t *shm_acc; // to communicate with db server

    // create semaphore
    sem_id = createSemaphore((key_t) SEM_KEY);
    sleep(1);

    // create message queue
    msgq_id = createMessage((key_t) MSG_KEY);

    // create shared mem
    shm_id = createSHM((key_t)SHM_KEY, sizeof(account_t));
    // attaching shared memory segment to get pointer to it
    shm_acc = (account_t *)accessSHM(shm_id);


    // loop forever and request for data from user
    while(loop){
        printf("Enter 5 digit account number: ");
        scanf(" %5s", temp_acc.accountNum);

        printf("Enter 3 digit PIN: ");
        scanf(" %3s", temp_acc.pin);

        printf("Enter funds available: ");
        scanf(" %f", &temp_acc.funds);


        // TODO check if inputs are valid


        // entering critical section, so call wait
        waitSemaphore(sem_id);

        // set new account values
        strcpy(shm_acc->accountNum, temp_acc.accountNum);
        strcpy(shm_acc->pin, temp_acc.pin);
        shm_acc->funds = temp_acc.funds;

        // once user enters their inputted info, signal semaphore
        signalSemaphore(sem_id);

        // sending message
        strcpy(send_message.msg, "UPDATE_DB");

        printf("Sending message: UPDATE_DB to db_server.\n");

        sendMessage(msgq_id, (void *)&send_message, sizeof(send_message.msg), 1);
        //printf("Data has been updated in database: \n%5s,%3s,%.2f\n\n", temp_acc.accountNum, temp_acc.pin, temp_acc.funds);

        //loop = false; // just let db_editor run once for now
    }
    kill(server_pid, SIGTERM);
    // destroy semaphore
    removeSemaphore(sem_id);
    // detach from shared memory
    detachSHM(shm_acc);
}

int main(){
   pid_t server_pid = fork();

   if (server_pid == 0) { // is child process
       execlp("./db_server", "db_server", NULL);

   } else if (server_pid > 0){ // is parent process
       run_editor(server_pid);

   } else if (server_pid < 0){
       fprintf(stderr, "Fork failed");
       exit(-1);
   }
}