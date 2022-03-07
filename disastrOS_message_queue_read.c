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
        running->syscall_retvalue = DSOS_ESYSCALL_NOT_IMPLEMENTED;
        return;
    }

    MessageQueue* mq = (MessageQueue*)resource;
    if(mq->msgs.size == 0) {
        running->status = Waiting;
        List_insert(&waiting_list, waiting_list.last, (ListItem*)running);
        List_insert(&mq->writing_pids, mq->writing_pids.last, (ListItem*) PCBPtr_alloc(running));

        PCB* next = (PCB*)List_detach(&ready_list, ready_list.first);
        next->status = Running;
        running = next;
        return;
    }

    Message* m = (Message*)mq->msgs.first;
    int l_message = m->length;
    if(m->length > length) {
        running->return_value = DSOS_MESSAGELENGTHNOTVALID;
        return;
    }
    
    for(int i = 0; i < m->length; i++) {
        msg_ptr[i] = m->msg[i];
    }

    List_detach(&mq->msgs, (ListItem*)m);

    assert(m_free(m)>=0);

    while(mq->writing_pids.size > 0) {
        PCBPtr* process = (PCBPtr*)List_detach(&mq->writing_pids, mq->writing_pids.first);
        PCB* pcb = process->pcb;

        pcb->status = Ready;
        pcb->return_value = DSOS_MQ_CONTINUE;
        List_insert(&ready_list, ready_list.last, (ListItem*) pcb);

        assert(PCBPtr_free(process)>=0);
    }

    running->return_value = l_message;
}