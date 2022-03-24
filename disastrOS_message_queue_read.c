#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "disastrOS_syscalls.h"
#include "disastrOS_message_queue.h"
#include "disastrOS_descriptor.h"
#include "disastrOS_pcb.h"

//read a message from a message queue.
void internal_message_queue_read() {
    int fd = running->syscall_args[0];
    char* msg_ptr = (char*)running->syscall_args[1];
    int length = running->syscall_args[2];

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
    if(mq->msgs.size == 0) {
        if(mq->writing_pid != NULL) {
            running->syscall_retvalue = DSOS_WAITINGFORMESSAGE;
        } else {
            running->syscall_retvalue = DSOS_MESSAGEQUEUEEMPTY;
        }
        return;
    }

    Message* m = (Message*)mq->msgs.first;
    
    if(m->length > length) {
        running->syscall_retvalue = DSOS_MESSAGELENGTHNOTVALID;
        return;
    } else {
        for(int i = 0; i < m->length; i++) {
            msg_ptr[i] = m->msg[i];
        }
        running->syscall_retvalue = m->length;
    }
    List_detach(&mq->msgs, (ListItem*)m);
    assert(m_free(m)>=0);
}