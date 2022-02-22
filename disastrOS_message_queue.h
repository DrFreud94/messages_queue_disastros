#include "disastrOS_resource.h"
#include "pool_allocator.h"

#define MESSAGE_LENGTH_MAX 128

typedef struct MessageQueue {
    Resource* resource;
    ListHead messages;
    ListHead waiting_write_pcbs;
    ListHead waiting_read_pcbs;
    int available;
}MessageQueue;

typedef struct Message {
    ListItem list; //extends ListItem
    int sender_pid;
    char message[MESSAGE_LENGTH_MAX];
    int length;
}Message;

//initialize the message queue
void mq_init();

//allocate a message queue
Resource* mq_alloc(int id);

//free a message queue
int mq_free(Resource* r);

//get the first message from queue
Message* mq_get_first_message(MessageQueue* mq);

//print the message queue
void print_mq(MessageQueue* mq);

//initialize Message at operating system level
void m_init();

//allocate a message
Message* m_alloc(int pid_sender, char* message, int length);

//free a message
int m_free(Message* m);
