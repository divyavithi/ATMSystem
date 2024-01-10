
#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <sys/msg.h>

int createMessage(key_t key);
void sendMessage(int msgid, void *send_message, int message_size, long msg_type);
void receiveMessage(int msgid, void *send_message, int message_size, long message_type);
void deleteMessage(int msgid);

#endif