
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

#include "structs.h"

int createMessage(key_t key){
    int msgqid = msgget(key, IPC_CREAT | 0666);
    if (msgqid == -1){
        perror("msgsnd");
        exit(1);
    }
    return msgqid;
}

// function to send a message
void sendMessage(int msgqid, void *send_message, int message_size, long msg_type) {
    if (msgsnd(msgqid, send_message, message_size, 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
}

// function to receive and display message
void receiveMessage(int msgqid, void *send_message, int message_size, long message_type) {

    if (msgrcv(msgqid, send_message, message_size, message_type, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }

}

void deleteMessage(int msgqid){
    if(msgqid != -1) { //IPC error
        msgctl(msgqid, IPC_RMID, 0);
    }
}