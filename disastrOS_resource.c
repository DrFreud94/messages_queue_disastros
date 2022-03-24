#include <assert.h>
#include <stdio.h>
#include "disastrOS_resource.h"
#include "disastrOS_descriptor.h"
#include "pool_allocator.h"
#include "linked_list.h"
#include "disastrOS_message_queue.h"
#include "disastrOS_resource_OS.h"

static Resource* (*alloc_func[MAX_TYPE_RESOURCES])();
static int (*dealloc_func[MAX_TYPE_RESOURCES])(Resource*);

static int (*set_mode[MAX_TYPE_RESOURCES])(Resource*, int);
static int (*delete_mode[MAX_TYPE_RESOURCES])(Resource*, int);



void Resource_init(){
    
    //alloc and dealloc resources of OS, standard
    alloc_func[STANDARD_RESOURCE_TYPE] = sr_alloc;
    dealloc_func[STANDARD_RESOURCE_TYPE] = sr_free;

    //alloc and dealloc mq
    alloc_func[MESSAGE_QUEUE_TYPE] = mq_alloc;
    dealloc_func[MESSAGE_QUEUE_TYPE] = mq_free;

    //set and delete opening mode for MQ
    set_mode[MESSAGE_QUEUE_TYPE] = mq_set_mode;
    delete_mode[MESSAGE_QUEUE_TYPE] = mq_delete_mode;
}

Resource* Resource_alloc(int id, int type){

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

int Resource_open(Resource* resource, int mode) {
  int result = 0;
  if(resource->type > STANDARD_RESOURCE_TYPE)
    result = (*set_mode[resource->type])(resource, mode);
  return result;
}

int Resource_release(Resource* r, int mode) {
  int result = 0;
  if(r->type > STANDARD_RESOURCE_TYPE)
    result = (*delete_mode[r->type])(r, mode);
  return result; 
}

int Resource_free(Resource* r) {
  if(r->type + 1 > MAX_TYPE_RESOURCES) {
    return DSOS_ERESOURCETYPENOTEXISTING;
  }
  assert(r->descriptors_ptrs.first==0);
  assert(r->descriptors_ptrs.last==0);
  return (*dealloc_func[r->type])(r);
}

Resource* ResourceList_byId(ResourceList* l, int id) {
  ListItem* aux=l->first;
  while(aux){
    Resource* r=(Resource*)aux;
    if (r->id==id) {
      return r;
    }
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
