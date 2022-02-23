#include "disastrOS_message_queue.h"
#include "pool_allocator.h"
#include "disastrOS_constants.h"

#include <assert.h>

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

//init Message allocator
void m_init() {
    int result = PoolAllocator_init(&_m_allocator, sizeof(Message), M_FOR_MQ, _m_ptr_buffer, M_BUFFER_SIZE);
    assert(!result);
}