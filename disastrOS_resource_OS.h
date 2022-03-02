#include "disastrOS_resource.h"

//struct for std resource disastrOS
typedef struct {
    Resource resource;
}StandardResource;

//init standard resource for disastrOS
void StandardResource_init();

//alloc standard resource for disastrOS
Resource* sr_alloc(int id, int type);

//dealloc standard resource for disastrOS
int sr_free(Resource* r);

