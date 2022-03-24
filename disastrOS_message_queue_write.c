#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include "disastrOS_syscalls.h"
#include "disastrOS_message_queue.h"
#include "disastrOS_descriptor.h"
#include "disastrOS_pcb.h"

//write a message in a message queue.
void internal_message_queue_write() {
    int fd = running->syscall_args[0];
    const char* msg_ptr = (char*)running->syscall_args[1];
    int length = running->syscall_args[2];

    if(length < 0 || length > MESSAGE_STRING_MAX_LENGTH) {
        running->syscall_retvalue = DSOS_MESSAGELENGTHNOTVALID;
        return;
    }

    Descriptor* d = DescriptorList_byFd(&running->descriptors, fd);
    if(d == NULL) {
        running->syscall_retvalue = DSOS_RESOURCENOTFOUND;
        return;
    }

    Resource* resource = d->resource;
    if(resource->type != MESSAGE_QUEUE_TYPE) {
        running->syscall_retvalue = DSOS_ERESOURCETYPEWRONG;
        return;
    }

    MessageQueue* mq = (MessageQueue*)resource;

    if(mq->msgs.size == M_FOR_MQ) {
        running->return_value = DSOS_MESSAGEQUEUEFULL;
        return;
    }
    
    Message* m = m_alloc(msg_ptr, length);
    List_insert(&mq->msgs, mq->msgs.last, (ListItem*)m);
    
    running->syscall_retvalue = length;
}