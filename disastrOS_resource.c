#include <assert.h>
#include <stdio.h>
#include "disastrOS_resource.h"
#include "disastrOS_descriptor.h"
#include "pool_allocator.h"
#include "linked_list.h"
#include "disastrOS_message_queue.h"
#include "disastrOS_resource_OS.h"

// #define RESOURCE_SIZE sizeof(Resource)
// #define RESOURCE_MEMSIZE (sizeof(Resource)+sizeof(int))
// #define RESOURCE_BUFFER_SIZE MAX_NUM_RESOURCES*RESOURCE_MEMSIZE

// static char _resources_buffer[RESOURCE_BUFFER_SIZE];
// static PoolAllocator _resources_allocator;

static Resource* (*alloc_func[MAX_TYPE_RESOURCES])();
static int (*dealloc_func[MAX_TYPE_RESOURCES])(Resource*);

void Resource_init(){
    // int result=PoolAllocator_init(& _resources_allocator,
		// 		  RESOURCE_SIZE,
		// 		  MAX_NUM_RESOURCES,
		// 		  _resources_buffer,
		// 		  RESOURCE_BUFFER_SIZE);
    // assert(! result);
    
    //alloc and dealloc resources of OS, standard
    alloc_func[STANDARD_RESOURCE_TYPE] = sr_alloc;
    dealloc_func[STANDARD_RESOURCE_TYPE] = sr_free;

    //alloc and dealloc mq
    alloc_func[MESSAGE_QUEUE_TYPE] = mq_alloc;
    dealloc_func[MESSAGE_QUEUE_TYPE] = mq_free;
}

Resource* Resource_alloc(int id, int type){
  // Resource* r=(Resource*) PoolAllocator_getBlock(&_resources_allocator);

  if(type + 1 > MAX_TYPE_RESOURCES) {
    return NULL;
  }

  Resource* r= (*alloc_func[type])();

  if (!r)
    return 0;
  r->list.prev=r->list.next=0;
  r->id=id;
  r->type=type;
  List_init(&r->descriptors_ptrs);
  return r;
}

int Resource_free(Resource* r) {
  if(r->type + 1 > MAX_TYPE_RESOURCES) {
    return DSOS_ERESOURCETYPENOTEXISTING;
  }
  assert(r->descriptors_ptrs.first==0);
  assert(r->descriptors_ptrs.last==0);
  //return PoolAllocator_releaseBlock(&_resources_allocator, r);
  return (*dealloc_func[r->type])(r);
}

Resource* ResourceList_byId(ResourceList* l, int id) {
  ListItem* aux=l->first;
  while(aux){
    Resource* r=(Resource*)aux;
    if (r->id==id)
      return r;
    aux=aux->next;
  }
  return 0;
}

void Resource_print(Resource* r) {
  printf("id: %d, type:%d, pids:", r->id, r->type);
  DescriptorPtrList_print(&r->descriptors_ptrs);
}

void ResourceList_print(ListHead* l){
  ListItem* aux=l->first;
  printf("{\n");
  while(aux){
    Resource* r=(Resource*)aux;
    printf("\t");
    Resource_print(r);
    if(aux->next)
      printf(",");
    printf("\n");
    aux=aux->next;
  }
  printf("}\n");
}
