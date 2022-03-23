#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>
#include <string.h>
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

void read_childFunction() {

}

void write_childFunction() {

}

void initFunction(void* args) {
  disastrOS_printStatus();
//   printf("hello, I am init and I just started\n");
//   disastrOS_spawn(sleeperFunction, 0);
  

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