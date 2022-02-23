#include "disastrOS_resource.h"
#include "disastrOS_pcb.h"

typedef struct {
    Resource resource;
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