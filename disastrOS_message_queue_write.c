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
    const char* msg_ptr = running->syscall_args[1];
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
        running->syscall_retvalue = DSOS_ESYSCALL_NOT_IMPLEMENTED;
        return;
    }

    MessageQueue* mq = (MessageQueue*)resource;

    if(mq->msgs.size == M_FOR_MQ) {
        running->status = Waiting;
        List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
        List_insert(&mq->writing_pids, mq->writing_pids.last, (ListItem*) PCBPtr_alloc(running));

        PCB* next = List_detach(&ready_list, ready_list.first);
        next->status = Running;
        running = next;
        return;
    }

    Message* m = m_alloc(msg_ptr, length, running->pid);

    List_insert(&mq->msgs, mq->msgs.last, (ListItem*)m);

    while(mq->reading_pids.size > 0) {
        PCBPtr* process = (PCBPtr*)List_detach(&mq->reading_pids, mq->reading_pids.first);
        PCB* pcb = process->pcb;

        pcb->status = Ready;
        pcb->return_value = DSOS_MQ_CONTINUE;
        List_insert(&ready_list, ready_list.last, (ListItem*) pcb);

        assert(PCBPtr_free(process)>=0);
    }

    running->syscall_retvalue = length;
}