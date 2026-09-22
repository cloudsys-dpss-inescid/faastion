#ifndef GRAAL_CAPI_H
#define GRAAL_CAPI_H

// Header: https://github.com/oracle/graal/blob/master/substratevm/src/com.oracle.svm.core/headers/graal_isolate.preamble
#include "graal_isolate.h"

// C-API: https://www.graalvm.org/latest/reference-manual/native-image/native-code-interoperability/C-API
typedef struct {
    int                 (*graal_create_isolate)     (graal_create_isolate_params_t*, graal_isolate_t**, graal_isolatethread_t**);
    int                 (*graal_tear_down_isolate)  (graal_isolatethread_t*);
    graal_isolate_t*    (*graal_get_isolate)        (graal_isolatethread_t*);
    void                (*entrypoint)               (graal_isolatethread_t*, const char*, const char*, unsigned long);
    int                 (*graal_detach_thread)      (graal_isolatethread_t*);
int                     (*graal_attach_thread)      (graal_isolate_t*, graal_isolatethread_t**);
} isolate_abi_t; // TODO - rename to svm_abi_t

#endif