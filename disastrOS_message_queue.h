#include "disastrOS_resource.h"
#include "disastrOS_pcb.h"
#include "linked_list.h"

#define MESSAGE_STRING_MAX_LENGTH 128

//MessageQueue struct
typedef struct {
    Resource resource;
    ListHead msgs;
    PCBPtr* reading_pid;
    PCBPtr* writing_pid;
}MessageQueue;

//Message struct
typedef struct {
    ListItem list;
    char msg[MESSAGE_STRING_MAX_LENGTH];
    int length;
}Message;

/*
 * MessageQueue function signatures. They execute the principal jobs about instatiation, allocation and deallocation.
 * I provide a print function, that take in input a MessageQueue instance, and print all the elements inside the queue.
 */

// instatiation at disastrOS start
void mq_init();

//allocation of MessageQueue instance. It returns a Resource, because MessageQueue has a parent struct, Resource.
Resource* mq_alloc();

//deallocation of MessageQueue instance
int mq_free(Resource* r);

//set mode for opening resource
int mq_set_mode(Resource* r, int mode);

//delete mode on close resource
int mq_delete_mode(Resource* r, int mode);

//print the queue
void print_mq(Resource* r);

//get first messages from queue
Message* getMessage(Resource* r);

//instantiation at disastrOS start, and message queue init
void m_init();

//allocation of message instance
Message* m_alloc(const char* msg, int length);

//deallocation of MessageQueue instance
int m_free(Message* m);
