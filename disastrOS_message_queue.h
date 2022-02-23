#include "disastrOS_resource.h"
#include "disastrOS_pcb.h"

#define MESSAGE_STRING_MAX_LENGTH 128

//MessageQueue struct
typedef struct {
    Resource resource;
    ListHead* msgs;
    ListHead* reading_pids;
    ListHead* writing_pids;
}MessageQueue;

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

//print the queue
void print_mq(Resource* r);


//Message struct
typedef struct {
    ListItem list;
    char msg[MESSAGE_STRING_MAX_LENGTH];
    int length;
    int sender_pid_id;
}Message;

//instantiation at disastrOS start, and message queue init
void init();

//allocation of message instance
Message* m_alloc(char* msg, int length, int sender_id);

//deallocation of MessageQueue instance
int m_free(Message* m);
