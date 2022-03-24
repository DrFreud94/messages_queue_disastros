#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include "disastrOS.h"
#include "disastrOS_message_queue.h"

//parameter runtime
//int run = 1;

//counters for reading and writing messages
int counters[2];

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void read_childFunction(void *args) {
  printf("Hello! I'm a reader function for Message queue (id = %d) - PID: %d\n", *(int*)args, disastrOS_getpid());
  printf("Opening MQ in read mode...\n");

  int fd = disastrOS_openResource(*(int*)args, MESSAGE_QUEUE_TYPE, DSOS_READ);
  assert(fd >= 0);
  printf("MQ opened in read mode - fd = %d.\n", fd);

  char message[MESSAGE_STRING_MAX_LENGTH];
  memset(message, 0, sizeof(message));
  int c = 1;
  int res = -1;
  while(c) {
    
    res = disastrOS_mq_read(fd, message, MESSAGE_STRING_MAX_LENGTH);
    
    switch(res) {
    
    case DSOS_WAITINGFORMESSAGE:
      printf("The queue is EMPTY! But there is a writer in waiting for... Awaiting...\n");
      disastrOS_printStatus();
      disastrOS_sleep((20-disastrOS_getpid())*3);
      break;

    case DSOS_MESSAGEQUEUEEMPTY:
      printf("The queue is EMPTY! There is no process that want to send a message! Finishing process...\n");
      disastrOS_printStatus();
      c = 0;
      break;

    case DSOS_MESSAGELENGTHNOTVALID:
      printf("The message length is not valid! Waiting for reading another message!\n");
      disastrOS_printStatus();
      disastrOS_sleep((20-disastrOS_getpid())*3);
      break;

    default:
      counters[1]++;
      printf("Message: %s - PID: %d\n", message, disastrOS_getpid());
      printf("STATUS => written messages: %d, read messages: %d\n", counters[0], counters[1]);
      disastrOS_printStatus();
      disastrOS_sleep((20-disastrOS_getpid())*3);
    }
  }

  // while(run) {
  //   disastrOS_printStatus();
  //   printf("writer log (PID: %d)\n", disastrOS_getpid());
  //   disastrOS_sleep((20-disastrOS_getpid())*5);
  // }
  printf("The reader finished his process - closing resource...\n");
  disastrOS_closeResource(fd, DSOS_READ);
  disastrOS_exit(disastrOS_getpid()+1);
}

void write_childFunction(void *args) {
  printf("Hello! I'm a writer function for Message queue (id = %d) - PID: %d\n", *(int*)args, disastrOS_getpid());
  printf("Opening MQ in write mode...\n");

  int fd = disastrOS_openResource(*(int*)args, MESSAGE_QUEUE_TYPE, DSOS_WRITE);
  assert(fd >= 0);

  printf("MQ opened in write mode - fd = %d.\n", fd);

  char message[MESSAGE_STRING_MAX_LENGTH];

  snprintf(message, MESSAGE_STRING_MAX_LENGTH, "Hi! I'm the writer PID: %d", disastrOS_getpid());

  int res = -1;
  for(int i = 0; i < 10; i++) {
    printf("ITERATION NUMBER : %d\n", i);
    res = disastrOS_mq_write(fd, message, MESSAGE_STRING_MAX_LENGTH);
    switch(res) {

    case DSOS_MESSAGELENGTHNOTVALID:
      printf("Message not valid - Too long for communicate\n");
      printf("STATUS => written messages: %d, read messages: %d\n", counters[0], counters[1]);
      disastrOS_printStatus();
      disastrOS_sleep((20-disastrOS_getpid())*3);
      break;

    case DSOS_RESOURCENOTFOUND:
      printf("Error! The desired resource is not found! Aborting...\n");
      i = M_FOR_MQ;
      break;

    case DSOS_ERESOURCETYPEWRONG:
      printf("Error! The desired resource with type %d is not found! Aborting...\n", MESSAGE_QUEUE_TYPE);
      i = M_FOR_MQ;
      break;

    case DSOS_MESSAGEQUEUEFULL:
      printf("The queue is FULL! Waiting for write the message...");
      i--;
      disastrOS_printStatus();
      disastrOS_sleep((20-disastrOS_getpid())*3);
      break;

    default:
      counters[0]++;
      printf("STATUS => written messages: %d, read messages: %d\n", counters[0], counters[1]);
      disastrOS_printStatus();
      disastrOS_sleep((20-disastrOS_getpid())*3);
    }
    // counters[0]++;
    // printf("STATUS => written messages: %d, read messages: %d\n");
    // disastrOS_printStatus();
    // disastrOS_sleep((20-disastrOS_getpid())*6);
  }

  // while(run) {
  //   disastrOS_printStatus();
  //   printf("writer log (PID: %d)\n", disastrOS_getpid());
  //   printf("***********************************************************************\n");
  //   disastrOS_sleep((20-disastrOS_getpid())*5);
  // }
  printf("The writer finished his process - closing resource...\n");
  disastrOS_closeResource(fd, DSOS_WRITE);
  disastrOS_exit(disastrOS_getpid()+1);
}

// void childFunction(void* args){
//   printf("Hello, I am the child function %d\n",disastrOS_getpid());
//   printf("I will iterate a bit, before terminating\n");
//   int type=0;
//   int mode=0;
//   int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
//   printf("fd=%d\n", fd);
//   printf("PID: %d, terminating\n", disastrOS_getpid());

//   for (int i=0; i<(disastrOS_getpid()+1); ++i){
//     printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
//     disastrOS_sleep((20-disastrOS_getpid())*5);
//   }
//   disastrOS_exit(disastrOS_getpid()+1);
// }

void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);

  printf("Creating a MessageQueue for more process!\n");
  int resources[2];
  resources[0] = disastrOS_openResource(0, MESSAGE_QUEUE_TYPE, DSOS_CREATE);
  assert(resources[0] >= 0);
  resources[1] = disastrOS_openResource(1, MESSAGE_QUEUE_TYPE, DSOS_CREATE);
  assert(resources[1] >= 0);

  printf("Message Queue created - ID/fd: %d\n", resources[0]);
  printf("Message Queue created - ID/fd: %d\n", resources[1]);


  int alive_children = 0;
  for(int i = 0; i < 2; i++) {
    disastrOS_spawn(write_childFunction, resources + i);
    disastrOS_spawn(read_childFunction, resources + i);
    alive_children += 2;
  }
  int retval = -1;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0) {
    printf("initFunction, child: %d terminated, retval: %d, alive: %d \n", pid, retval, alive_children);
    --alive_children;
  }

//   printf("I feel like to spawn 10 nice threads\n");
//   int alive_children=0;
//   for (int i=0; i<10; ++i) {
//     int type=0;
//     int mode=DSOS_CREATE;
//     printf("mode: %d\n", mode);
//     printf("opening resource (and creating if necessary)\n");
//     int fd=disastrOS_openResource(i,type,mode);
//     printf("fd=%d\n", fd);
//     disastrOS_spawn(childFunction, 0);
//     alive_children++;
//   }

//   disastrOS_printStatus();
//   int retval;
//   int pid;
//   while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
//     disastrOS_printStatus();
//     printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
// 	   pid, retval, alive_children);
//     --alive_children;
//   }
  printf("Written messages: %d\n", counters[0]);
  printf("readed messages: %d\n", counters[1]);

  for(int i = 0; i < 2; i++) {
    disastrOS_destroyResource(resources[i]);
  }
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer read_childFunction is: %p\n", read_childFunction);
  printf("the function pointer write_childFunction is: %p\n", write_childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}