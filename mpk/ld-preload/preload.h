#include "utils/appmap.h"
#include "utils/threadmap.h"
#include "helpers/helpers.h"
#include "../erim/common.h"
#include "../erim/erim.h"

#define MALLOC
#define REALLOC
#define FREE
#define MMAP
#define MUNMAP
#define DLOPEN
#define PTHREAD_CREATE
#define PTHREAD_EXIT

void setAppPermissions(const char* id, int protectionFlag, int pkey);
int isEmpty(int domain);

