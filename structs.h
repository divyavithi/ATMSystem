/**
* To ensure entire system uses the same structs
*/

#ifndef STRUCTS_H
#define STRUCTS_H

/**
 * ATM messaging/transferring data across processes
 */
typedef struct message{
    long int msg_type; // 1, 2, 3, 4 (corresponding to message below)
    char msg[20]; // "PIN", "BALANCE", "WITHDRAW", "UPDATE_DB"
    char status[20];
    char accountNum[6];
    char pin[4];
    char amount[20];
    float inputtedWithdraw;
    float funds;
} message_t;

/**
* Customers accounts in DB
*/
typedef struct account{
    int numAccounts; // number of accounts from DB
    char accountNum[6]; // 5 chars + 1 null terminator
    char pin[4]; // 3 chars + 1 null terminator
    float funds;
    float withdraw;
    int attempts;
    int blocked;
} account_t;

#endif