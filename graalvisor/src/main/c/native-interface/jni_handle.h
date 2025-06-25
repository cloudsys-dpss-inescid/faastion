#ifndef __JNI_HANDLE_H__
#define __JNI_HANDLE_H__

#include "memory_map.h"

int load_native_method(IsolateFunction *function, const char *symbol);
void load_native_library(IsolateFunction *function, const char *filename);

extern void *_native_method;

#endif // __JNI_HANDLE_H__