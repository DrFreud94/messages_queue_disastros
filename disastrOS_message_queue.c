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



/***************************************************************************MESSAGE*********************************/
void m_init() {
    int result = PoolAllocator_init(&_m_allocator, M_SIZE, M_FOR_MQ, _m_buffer, M_BUFFER_SIZE);
    assert(!result);
}

