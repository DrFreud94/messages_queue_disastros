#include "disastrOS_resource_OS.h"
#include "disastrOS_resource.h"
#include "pool_allocator.h"

#include <assert.h>

#define STANDARD_RESOURCE_SIZE sizeof(StandardResource)
#define STANDARD_RESOURCE_MEMSIZE (sizeof(StandardResource)+sizeof(int))
#define STANDARD_RESOURCE_BUFFER_SIZE MAX_NUM_RESOURCES*STANDARD_RESOURCE_MEMSIZE

static char _standard_resources_buffer[STANDARD_RESOURCE_BUFFER_SIZE];
static PoolAllocator _standard_resources_allocator;

void StandardResource_init() {
    int result=PoolAllocator_init(& _standard_resources_allocator,
		STANDARD_RESOURCE_SIZE,
		MAX_NUM_RESOURCES,
		_standard_resources_buffer,
		STANDARD_RESOURCE_BUFFER_SIZE);
    assert(! result);
}

Resource* sr_alloc(int id, int type) {
    StandardResource* r=(StandardResource*) PoolAllocator_getBlock(&_standard_resources_allocator);
    if (!r)
        return 0;

    return (Resource*)r;
}

int sr_free(Resource* r) {
    return PoolAllocator_releaseBlock(&_standard_resources_allocator, (StandardResource*) r);
}