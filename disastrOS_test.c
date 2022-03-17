#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <assert.h>

#include "disastrOS.h"
#include "disastrOS_message_queue.h"

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;
  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);
  printf("PID: %d, terminating\n", disastrOS_getpid());

  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    disastrOS_sleep((20-disastrOS_getpid())*5);
  }
  disastrOS_exit(disastrOS_getpid()+1);
}

void read_childFunction(void* args) {
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=MESSAGE_QUEUE_TYPE;
  int mode=0;
  int fd_passed = *(int*)args;
  int fd=disastrOS_openResource(fd_passed, type, mode);
  assert(fd >= 0);
  printf("reading MessageQueue fd=%d\n process PID: %d\n", fd, disastrOS_getpid());

  char message[MESSAGE_STRING_MAX_LENGTH];
  
  int messages_read = 0;
  int ret_value = DSOS_MQ_CONTINUE;

  while(ret_value) {
    memset(message, 0, sizeof(message));
    ret_value = disastrOS_mq_read(fd, message, MESSAGE_STRING_MAX_LENGTH);
    if(ret_value > 0) {
      messages_read++;
      printf("child %d read: %s.\n", disastrOS_getpid(), message);
    }
  }

  printf("child %d - read %d messages.\n", disastrOS_getpid(), messages_read);
  disastrOS_closeResource(fd);
  disastrOS_exit(messages_read);
}

void write_childFunction(void* args) {
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=MESSAGE_QUEUE_TYPE;
  int mode=0;
  int fd_passed = *(int*)args;
  int fd=disastrOS_openResource(fd_passed, type, mode);
  printf("writing MessageQueue fd=%d\n process PID: %d\n", fd, disastrOS_getpid());

  disastrOS_printStatus();

  char message[MESSAGE_STRING_MAX_LENGTH] = "Hello, I'm a writer\n";

  int ret = DSOS_MQ_CONTINUE;
  int messages_written = 0;

  while(ret >= 0) {
    ret = disastrOS_mq_write(fd, message, MESSAGE_STRING_MAX_LENGTH);
    // printf("%d\n", ret);
    assert(ret >= 0);
    messages_written++;
  }
  printf("child %d - read %d messages.\n", disastrOS_getpid(), messages_written);
  disastrOS_closeResource(fd);
  disastrOS_exit(messages_written);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  // disastrOS_spawn(sleeperFunction, 0);
  

  printf("I feel like to spawn 4 nice threads\n");
  int alive_children=0;
  int resources[2];

  int type=MESSAGE_QUEUE_TYPE;
  int mode=DSOS_CREATE;
  printf("mode: %d\n", mode);
  printf("opening resource (and creating if necessary)\n");
  for (int i=0; i<2; ++i) {
    // int type=0;
    // int mode=DSOS_CREATE;
    // printf("mode: %d\n", mode);
    // printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    assert(fd >= 0);
    printf("fd=%d\n", fd);
    resources[i] = fd;
    // disastrOS_spawn(childFunction, 0);
    // alive_children++;
  }

  for(int i = 0; i < 2; ++i) {
    disastrOS_spawn(write_childFunction, &resources[i]);
    disastrOS_spawn(read_childFunction, &resources[i]);

    alive_children += 2;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  int messages_readed = 0;
  int messages_written = 0;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);

    if(pid%2 == 0) messages_written += retval;
    else messages_readed += retval;

    --alive_children;
  }


  printf("Messages readed: %d\n", messages_readed);
  printf("Messages written: %d\n", messages_written);

  for(int i = 0; i < 2; ++i) {
    disastrOS_closeResource(resources[i]);
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
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
