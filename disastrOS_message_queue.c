#include "disastrOS_message_queue.h"
#include "pool_allocator.h"
#include "disastrOS_constants.h"
#include "disastrOS_globals.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define MQ_SIZE sizeof(MessageQueue)
#define MQ_MEMSIZE (sizeof(MessageQueue)+sizeof(int))
#define MQ_BUFFER_SIZE MQ_MEMSIZE*MAX_NUM_RESOURCES

#define M_SIZE sizeof(Message)
#define M_MEMSIZE (sizeof(Message)+sizeof(int))
#define M_TOTAL M_MEMSIZE*M_FOR_MQ
#define M_BUFFER_SIZE M_TOTAL*MAX_NUM_RESOURCES

//allocator for mq
static PoolAllocator _mq_allocator;
//memory buffer for mq allocator
static char _mq_ptr_buffer[MQ_BUFFER_SIZE];

//allocator for m
static PoolAllocator _m_allocator;
//memory buffer for m allocator;
static char _m_ptr_buffer[M_BUFFER_SIZE];

//init MessageQueue allocator
void mq_init() {
    int result = PoolAllocator_init(&_mq_allocator, sizeof(MessageQueue), MAX_NUM_RESOURCES, _mq_ptr_buffer, MQ_BUFFER_SIZE);
    assert(!result);
    m_init();
}

//alloc MessageQueue
Resource* mq_alloc() {
    MessageQueue* mq = PoolAllocator_getBlock(&_mq_allocator);

    if(!mq)
        return NULL;
    
    List_init(&mq->msgs);
    return (Resource*)mq;
}

//dealloc MessageQueue
int mq_free(Resource* r) {
    MessageQueue* mq = (MessageQueue*)r;
    ListItem* message = mq->msgs.first;
    int result = 0;
    while(message != NULL) {
        result = m_free((Message*)message);
        assert(!result);
        message = message->next;
    }
    return PoolAllocator_releaseBlock(&_mq_allocator, r);
}

//set mode
int mq_set_mode (Resource* r, int mode) {
    if(r->type != MESSAGE_QUEUE_TYPE) {
        return 0;
    }
    MessageQueue* mq = (MessageQueue*)r;
    
    if(mode == DSOS_READ) {
        if(mq->reading_pid != NULL) {
            return 0;
        }
        mq->reading_pid = PCBPtr_alloc(running);
    }
    else if(mode == DSOS_WRITE) { 
        if(mq->writing_pid != NULL) {
            return 0;
        }
        mq->writing_pid = PCBPtr_alloc(running);
    }
    return 1;
}

//delete mode
int mq_delete_mode(Resource* r, int mode) {
    if(r->type != MESSAGE_QUEUE_TYPE) {
        return 0;
    }
    MessageQueue* mq = (MessageQueue*)r;
    if(mode == DSOS_READ) mq->reading_pid = NULL;
    else if(mode == DSOS_WRITE) mq->writing_pid = NULL;

    return 1;
}

//print queue
void print_mq(Resource* r) {
    MessageQueue* mq = (MessageQueue*)r;
    printf("printing message queue with id %d\n", r->id);
    if(mq->msgs.size == 0) {
        printf("There is no message inside the queue!\n");
        printf("----------------------------------------------------------\n");
    } else {
        ListItem* messages = mq->msgs.first;
        int i = 0;
        while(messages != NULL) {
            printf("Message n. %d: \n", i);
            printf("%s\n",((Message*)messages)->msg);
            printf("-------------------------------------------------------\n");
            messages = messages->next;
            i = i + 1;
        }
        printf("----------------------------------------------------------\n");
    }

    if(mq->writing_pid == NULL) {
        printf("There is no writing pid for this MQ!\n");
    } else {
        PCB* write_pid = mq->writing_pid->pcb;
        printf("PID for writing n. %d;\n", write_pid->pid);
    }

    printf("----------------------------------------------------------\n");

    if(mq->reading_pid == NULL) {
        printf("There is no reading pid for this MQ!\n");
    } else {
        PCB* read_pid = mq->reading_pid->pcb;
        printf("PID for reading n. %d;\n", read_pid->pid);
    }
    printf("**********************************************************\n");
}

//get first message from queue
Message* getMessage(Resource* r) {
    if(r == NULL) return NULL;
    return (Message*)((MessageQueue*)r)->msgs.first;
}

//init Message allocator
void m_init() {
    int result = PoolAllocator_init(&_m_allocator, sizeof(Message), MAX_NUM_RESOURCES, _m_ptr_buffer, M_BUFFER_SIZE);
    assert(!result);
}

//alloc Message
Message* m_alloc(const char* msg, int length) {
    Message* m = PoolAllocator_getBlock(&_m_allocator);
    if(!m)
        return NULL;
    
    if(length > MESSAGE_STRING_MAX_LENGTH) {
        return NULL;
    }

    for(int i = 0; i < length; i++) {
        m->msg[i] = msg[i];
    }
    m->length = length;
    return m;
}

//dealloc Message
int m_free(Message* m) {
    return PoolAllocator_releaseBlock(&_m_allocator, m);
}