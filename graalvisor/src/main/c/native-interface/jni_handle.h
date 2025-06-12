#ifndef __JNI_HANDLE_H__
#define __JNI_HANDLE_H__

#include "memory_map.h"

int load_native_method(IsolateFunction *function, const char *filename, const char *symbol);

extern void *_native_method;

#endif // __JNI_HANDLE_H__