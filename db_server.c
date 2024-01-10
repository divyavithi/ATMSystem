#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/msg.h>
#include "semaphore.h"
#include "structs.h"
#include "msg_queue.h"
#include "shared_mem.h"
#include <stdbool.h>

#define SHM_KEY 1234
#define MSG_KEY 5678
#define SEM_KEY 2023
#define DB "database.csv"

// Function to verify PIN and update account status
void verifyPIN(account_t *accounts, char accountNum[], char pin[], message_t *send_message) {
    int found = 0;

    for (int i = 0; i < accounts->numAccounts; i++) {
        // Search account number in DB
        if (strcmp(accounts[i].accountNum, accountNum) == 0) {
            int encryptedPIN = atoi(pin) - 1;

            // Compare encryptedPIN with PIN in DB
            if (encryptedPIN == atoi(accounts[i].pin)) {
                strcpy(send_message->status, "PIN_OK");
            } else {
                // If failure on PIN
                strcpy(send_message->status, "PIN_WRONG");
                accounts[i].attempts++; // Increment failed attempts

                // Blocking account
                if (accounts[i].attempts >= 3) {
                    accounts[i].blocked = 1; // Blocked becomes true
                    accounts[i].accountNum[0] = 'X'; // First digit is converted to an 'X' char
                    printf("Account is blocked.\n");
                }
            }

            found = 1;
            break; // No need to continue searching
        }
    }

    // If account not found, send appropriate message
    if (!found) {
        strcpy(send_message->status, "ACCOUNT_NOT_FOUND");
    }
}

float getBalance(account_t *accounts, char accountNum[]) {
    for (int i = 0; i < accounts->numAccounts; i++) {
        // search account number in DB
        if (strcmp(accounts[i].accountNum, accountNum) == 0) {
            return accounts[i].funds;
        }
    }
    printf("No account found.\n");
    return -1;
}

// Function to handle withdrawal and update account status
void withdraw(account_t *accounts, char accountNum[], float withdrawalAmt, message_t *send_message) {
    int found = 0;

    for (int i = 0; i < accounts->numAccounts; i++) {
        // Search account number in DB
        if (strcmp(accounts[i].accountNum, accountNum) == 0) {
            // Check if there are enough funds
            if (accounts[i].funds >= withdrawalAmt) {
                // Sufficient funds
                strcpy(send_message->status, "FUNDS_OK");
                accounts[i].funds -= withdrawalAmt;

            } else {
                // Insufficient funds
                strcpy(send_message->status, "NSF");
            }

            found = 1;
            break; // No need to continue searching
        }
    }

    // If account not found, send appropriate message
    if (!found) {
        strcpy(send_message->status, "ACCOUNT_NOT_FOUND");
    }
}

// Function to update account and send response through message queue
void updateAccount(account_t *accounts, int msgq_id, char accountNum[], char pin[], message_t *received_message) {
    message_t send_message;

    // Verify PIN
    verifyPIN(accounts, accountNum, pin, &send_message);

    // If PIN verification is successful, check the message type
    if (strcmp(send_message.status, "PIN_OK") == 0) {
        // Handle withdrawal if the message is "WITHDRAW"
        if (strcmp(received_message->status, "WITHDRAW") == 0) {
            float withdrawalAmt = atof(received_message->amount);
            withdraw(accounts, accountNum, withdrawalAmt, &send_message);
        }
        // Add other message type checks here if needed
    }

    // Send the response message through the message queue
    sendMessage(msgq_id, &send_message, sizeof(send_message), 1);
}


// Function to update database
void updateDB(char accountNum[], char pin[], float funds) {
    char currAccNum[6];
    char currPIN[4];
    float currFunds;
    char line[100]; // for csv
    int found = 0;

    // read DB file
    FILE *file = fopen(DB, "r");
    if (file == NULL) {
        perror("Error opening file.\n");
        exit(1);
    }

    // make a temp file to store new values
    FILE *temp_file = fopen("temp_db.csv", "w");
    if (temp_file == NULL) {
        perror("Error creating file.\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), file) != NULL) { //TODO i think printing error is caused by this line

        sscanf(line, "%5s\t%3s\t%f", currAccNum, currPIN, &currFunds);

        if (strcmp(currAccNum, accountNum) == 0) { // account number found

            // subtract 1 from pin to encrypt
            int encryptedPIN = atoi(pin) - 1;
            char encryptedPINc[4];
            sprintf(encryptedPINc, "%d", encryptedPIN);

            // update pin and funds
            fprintf(temp_file,"%s\t%s\t%.2f\n",  accountNum, encryptedPINc, funds);

            found = 1;
            printf("Database updated.\n");
        } else { // copy unchanged data to temp file to show updated db
            //TODO FIX OUTPUT IN TEMP FILE (printed more lines than needed)
            fprintf(temp_file, "%s\t%s\t%.2f\n", currAccNum, currPIN, currFunds);
        }
    }
    fclose(file);
    fclose(temp_file);

    if (!found) {
        printf("Account not found in database.\n");
    }
}

int main() {
    int shm_id, sem_id, msgq_id;
    message_t received_message;
    account_t temp_acc;
    account_t *shm_acc;

    // create message queue
    msgq_id = createMessage((key_t) MSG_KEY); // assigned random key

    // create shared mem
    shm_id = createSHM((key_t) SHM_KEY, sizeof(account_t));
    // attaching shared memory segment to get pointer to it
    shm_acc = (account_t *) accessSHM(shm_id);

    // create semaphore
    sem_id = createSemaphore((key_t) SEM_KEY);
    waitSemaphore(sem_id); //wait

    while (1) {

        // receive message
        receiveMessage(msgq_id, (void *) &received_message, sizeof(received_message), -5);

        if (received_message.msg_type == 1) { // PIN = msg_type = 1
            // entering critical
            // entering critical section, so call wait
            waitSemaphore(sem_id);

            // copy obtained info from user into account
            strcpy(temp_acc.accountNum, shm_acc->accountNum);
            strcpy(temp_acc.pin, shm_acc->pin);

            // process complete, notify atm
            signalSemaphore(sem_id);

            updateAccount(shm_acc, msgq_id, temp_acc.accountNum, temp_acc.pin, &received_message);

        } else if (received_message.msg_type == 2) { // BALANCE = msg_type = 2
            // entering critical section, so call wait
            waitSemaphore(sem_id);

            // copy obtained info from user into account
            strcpy(temp_acc.accountNum, shm_acc->accountNum);
            strcpy(temp_acc.pin, shm_acc->pin);

            // process complete, notify atm
            signalSemaphore(sem_id);

            float acc_balance = getBalance(shm_acc, temp_acc.accountNum);

            // Prepare and send the response message through the message queue
            message_t send_balance_message;
            if (acc_balance >= 0) {
                sprintf(send_balance_message.status, "BALANCE_OK %.2f", acc_balance);
            } else {
                strcpy(send_balance_message.status, "ACCOUNT_NOT_FOUND");
            }

            sendMessage(msgq_id, &send_balance_message, sizeof(send_balance_message), 1);

        } else if (received_message.msg_type == 3) { // WITHDRAW = msg_type = 3
            // entering critical section, so call wait
            waitSemaphore(sem_id);

            // copy obtained info from user into account
            strcpy(temp_acc.accountNum, shm_acc->accountNum);
            strcpy(temp_acc.pin, shm_acc->pin);

            // process complete, notify atm
            signalSemaphore(sem_id);

            // Get the amount requested from the received message
            float withdrawalAmt = atof(received_message.amount);

            // Invoke the withdraw function and display user's balance
            withdraw(shm_acc, temp_acc.accountNum, withdrawalAmt, &received_message);

            // Send the response message through the message queue
            sendMessage(msgq_id, &received_message, sizeof(received_message), 1);

        } else if (received_message.msg_type == 4) { // UPDATE_DB = msg_type = 4 (defined in struct.h)
            // entering critical section, so call wait
            waitSemaphore(sem_id);

            // copy obtained info into account
            strcpy(temp_acc.accountNum, shm_acc->accountNum);
            strcpy(temp_acc.pin, shm_acc->pin);
            temp_acc.funds = shm_acc->funds;

            // shared memory complete, notify editor
            signalSemaphore(sem_id);

            updateDB(temp_acc.accountNum, temp_acc.pin, temp_acc.funds);
        }
    }

    // destroy semaphore
    semctl(sem_id, 0, IPC_RMID);
    // detach from shared memory
    detachSHM(shm_acc);

    return 0;
}
