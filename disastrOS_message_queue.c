#include <assert.h>
#include <stdio.h>

#include "disastrOS_message_queue.h"
#include "pool_allocator.h"
#include "disastrOS.h"
#include "disastrOS_globals.h"

#define MQ_SIZE sizeof(MessageQueue)
#define MQ_MEMSIZE (sizeof(MessageQueue)+sizeof(int))
#define MQ_BUFFER_SIZE MAX_NUM_RESOURCES * MQ_MEMSIZE

#define M_SIZE sizeof(Message)
#define M_MEMSIZE (sizeof(Message) + sizeof(int))
#define M_BUFFER_SIZE M_FOR_MQ * M_MEMSIZE

static PoolAllocator _mq_allocator;
static char _mq_buffer[MQ_BUFFER_SIZE];

static PoolAllocator _m_allocator;
static char _m_buffer[M_BUFFER_SIZE];

void mq_init() {
    int result = PoolAllocator_init(&_mq_allocator, MQ_SIZE, MAX_NUM_RESOURCES, _mq_buffer, MQ_BUFFER_SIZE);
    assert(!result);
    m_init();
}

Resource* mq_alloc(int id) {
    MessageQueue* mq = (MessageQueue*) PoolAllocator_getBlock(&_mq_allocator);
    if(mq == NULL) {
        return NULL;
    }
    mq->resource = Resource_alloc(id, MESSAGE_QUEUE_TYPE);
    List_init(&mq->messages);
    List_init(&mq->waiting_write_pcbs);
    List_init(&mq->waiting_read_pcbs);
    mq->available = 0;

    return (Resource*)mq;

}

int mq_free(Resource* r) {
    MessageQueue* mq = (MessageQueue*) r;
    int r_result = 0;
    ListItem* m = mq->messages.first;
    while (m != NULL) {
        r_result = m_free((Message*)m);
        if (r_result < 0) return r_result;
        m = m -> next;
    }
    return PoolAllocator_releaseBlock(&_mq_allocator, mq);
}

Message* mq_get_first_message(MessageQueue* mq) {
    if(mq == NULL) return NULL;
    return (Message*) mq->messages.first;
}

void print_mq(MessageQueue* mq) {
    if(mq == NULL) return;

    printf("Message queue - ID %d\n", mq->resource->id);
    ListItem* m = mq->messages.first;
    if(m == NULL) printf("Empty message queue.\n");

    int i = 0;
    while(m) {
        printf("Message %d: %s \n", i, (char*)(((Message*)m)->message));
        i++;
        m = m -> next;
    }

    ListItem* waiting_write = mq->waiting_write_pcbs.first;
    ListItem* waiting_read = mq->waiting_read_pcbs.first;

    while(waiting_write) {
        printf("Waiting writer - PID %d\n", ((PCBPtr*)waiting_write)->pcb->pid);
        waiting_write = waiting_write ->next;
    }

    while(waiting_read) {
        printf("Waiting reader - %d\n", ((PCBPtr*)waiting_read)->pcb->pid);
    }
}

/***************************************************************************MESSAGE*********************************/
void m_init() {
    int result = PoolAllocator_init(&_m_allocator, M_SIZE, M_FOR_MQ, _m_buffer, M_BUFFER_SIZE);
    assert(!result);
}

Message* m_alloc(int pid_sender, char* message, int length) {
    Message* m = (Message*) PoolAllocator_getBlock(&_m_allocator);
    if(m == NULL) return NULL;

    List_init(&m->list);
    m->sender_pid = pid_sender;
    for(int i = 0; i < length; i++) {
        m->message[i] = message[i];
    }
    m->length = length;

    return m;
}

int m_free(Message* m) {
    return PoolAllocator_releaseBlock(&_m_allocator, m);
}

