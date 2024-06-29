#ifndef __GRAALVISOR_H
#define __GRAALVISOR_H
#include "graal_isolate.h"
#include <sys/types.h>

typedef struct graal_visor_struct {
  void * (*f_host_receive_string)(graal_isolatethread_t *, char*);
} graal_visor_t;


#endif
