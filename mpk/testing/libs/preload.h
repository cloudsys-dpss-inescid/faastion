#ifndef PRELOAD_H
#define PRELOAD_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstring>
#include <dlfcn.h>
#include <iostream>
#include <link.h>
#include <list>
#include <map>
#include <pthread.h>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <tuple>

#include "../common/common.h"
#include "../erim/erim.h"

struct lib_info {
    const char* lib_name;
    const char* app_id;
};

extern std::map<std::string, std::list<std::tuple<void*, size_t>>> apps;

#endif // PRELOAD_H
