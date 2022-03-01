#include "disastrOS_message_queue.h"
#include "pool_allocator.h"
#include "disastrOS_constants.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define MQ_SIZE sizeof(MessageQueue)
#define MQ_MEMSIZE (sizeof(MessageQueue)+sizeof(int))
#define MQ_BUFFER_SIZE MQ_MEMSIZE*MAX_NUM_RESOURCES

#define M_SIZE sizeof(Message)
#define M_MEMSIZE (sizeof(Message)+sizeof(int))
#define M_BUFFER_SIZE M_MEMSIZE*M_FOR_MQ

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
    List_init(mq->msgs);
    List_init(mq->reading_pids);
    List_init(mq->writing_pids);

    return (Resource*)mq;
}

//dealloc MessageQueue
int mq_free(Resource* r) {
    MessageQueue* mq = (MessageQueue*)r;
    ListItem* message = mq->msgs->first;
    while(message != NULL) {
        int result = m_free((Message*)message);
        assert(!result);
        message = message->next;
    }
    return PoolAllocator_releaseBlock(&_mq_allocator, r);
}

//print queue
void print_mq(Resource* r) {
    MessageQueue* mq = (MessageQueue*)r;
    printf("printing message queue with id %d\n", r->id);
    
    ListItem* messages = mq->msgs->first;
    int i = 0;
    while(messages != NULL) {
        printf("Message n. %d: \n", i);
        printf("%s\n",((Message*)messages)->msg);
        printf("Sender PID: %d\n", ((Message*)messages)->sender_pid_id);
        printf("-------------------------------------------------------\n");
        messages = messages->next;
        i = i + 1;
    }

    ListItem* write_pids = mq->writing_pids;
    while(write_pids != NULL) {
        printf("PID waiting for writing n. %d;\n", ((PCB*)write_pids)->pid);
        write_pids = write_pids->next;
    }
    ListItem* read_pids = mq->reading_pids;
    while(read_pids != NULL) {
        printf("PID waiting for reading n. %d;\n", ((PCB*)read_pids)->pid);
        read_pids = read_pids->next;
    }
}

//init Message allocator
void m_init() {
    int result = PoolAllocator_init(&_m_allocator, sizeof(Message), M_FOR_MQ, _m_ptr_buffer, M_BUFFER_SIZE);
    assert(!result);
}

//alloc Message
Message* m_alloc(char* msg, int length, int sender_id) {
    Message* m = PoolAllocator_getBlock(&_m_allocator);
    for(int i = 0; i < length; i++) {
        m->msg[i] = msg[i];
    }
    m->length = length;
    m->sender_pid_id = sender_id;
    return m;
}

//dealloc Message
int m_free(Message* m) {
    return PoolAllocator_releaseBlock(&_m_allocator, m);
}