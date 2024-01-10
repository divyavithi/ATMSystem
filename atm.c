/**
 * Assignment 3 - Concurrency, IPC,
 * Semaphores, Shared and Virtual Memory, Files
 *
 * Part B - ATM Component Solution
 *
 * Created by Divya Vithiyatharan and Claire Villanueva
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "semaphore.h"
#include "structs.h"
#include "msg_queue.h"
#include "shared_mem.h"

#define SHM_KEY 1234
#define MSG_KEY 5678
#define SEM_KEY 2023

void runATM(pid_t pid) {
    int shm_id, sem_id, msgq_id;
    bool loop = true;
    message_t send_message;
    message_t received_message;


    // create message queue
    msgq_id = createMessage((key_t) MSG_KEY);

    // create shared mem
    shm_id = createSHM((key_t) SHM_KEY, sizeof(account_t));
    // attaching shared memory segment to get pointer to it
    //shm_acc = (account_t *)accessSHM(shm_id);

    // create semaphore
    sem_id = createSemaphore((key_t) SEM_KEY);
    signalSemaphore(sem_id); // get access


    while (loop) { // loop will become false when user enters 'x'

        // request account info
        printf("Enter 5 digit account number (or 'X' to exit): \n");
        scanf(" %5s", send_message.accountNum);

        if (strcmp(send_message.accountNum, "X") == 0) {
            break; // exit while loop
        }
        //TODO assign message type for send_message?
        int pin_attempts = 0;

        while (pin_attempts < 3) { // user has 3 attempts to input correct pin before account becomes blocked
            printf("Enter 3 digit PIN: ");
            scanf(" %3s", send_message.pin);

            // send PIN message to db server
            strcpy(send_message.msg, "PIN");
            sendMessage(msgq_id, (void *) &send_message, sizeof(send_message), 1);

            // wait to receive message from db server (PIN_WRONG or OK)
            receiveMessage(msgq_id, (void *) &received_message, sizeof(send_message), 5);

            // check if pin is correct
            if (strcmp(received_message.pin, "PIN_WRONG") == 0) {
                printf("Incorrect PIN. Please try again.\n");
                continue; // restart while loop

            } else if(strcmp(received_message.pin, "ACCOUNT_BLOCKED") == 0) {
                break; // exit while loop
            }
        }

    }
}

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        runATM(pid);
    } else if (pid > 0) {
        // db editor
    } else if (pid < 0) {
        fprintf(stderr, "Fork failed");
        exit(-1);
    }
}


