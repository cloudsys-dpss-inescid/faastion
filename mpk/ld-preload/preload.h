#include "utils/appmap.h"
#include "utils/threadmap.h"
#include "helpers/helpers.h"
#include <common.h>
#include <erim.h>

#define DLOPEN
#define PTHREAD_CREATE
#define PTHREAD_EXIT

void setAppPermissions(const char* id, int protectionFlag, int pkey);
int isEmpty(int domain);
int findEmptyDomain();
