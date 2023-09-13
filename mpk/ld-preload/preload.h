#include "utils/appmap.h"
#include "utils/threadmap.h"
#include "helpers/helpers.h"
#include <common.h>
#include <erim.h>

#define DLOPEN

/* Thread map methods */
void insertThreadInMap(int domain);
void removeThreadFromMap(int domain);
int findEmptyDomain();

/* App array methods (Lazy loading) */
void insertApp(int domain, const char* id);
int findApp(const char* id);
char* getApp(const char* domain);

/* */
void setAppPermissions(const char* id, int protectionFlag, int pkey);