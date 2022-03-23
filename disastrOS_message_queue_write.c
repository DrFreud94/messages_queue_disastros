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
    int pid_receiver = running->syscall_args[3];

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

    //TODO: impedire di inviare un messaggio se non ci sono processi che leggono.
    //Qualora ci sono, inviare lo stesso messaggio a tutti i processi, uno alla volta.

    if(mq->reading_pids.size == 0) {
        running->return_value=DSOS_EREADINGPCBMQNOTFOUND;
        return;
    }

    int messages_sent = 0;

    ListItem* process_reading = mq->reading_pids.first;
    while(process_reading != NULL) {
        Message* m = m_alloc(msg_ptr, length, running->pid, ((PCB*)process_reading)->pid);
        process_reading = process_reading->next;
        List_insert(&mq->msgs, mq->msgs.last, (ListItem*)m);
        messages_sent++;
    }
    running->syscall_retvalue = messages_sent;
}