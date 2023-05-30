#include "preload.h"

std::unordered_map<std::string, std::vector<MemoryRegion>> apps;
std::unordered_map<int, std::vector<pthread_t>> runningThreads;
std::mutex runningThreadsMutex;

static __thread int no_hook = 0;


/* Function pointers declarations */
static int( * real_pthread_create)(pthread_t * , const pthread_attr_t * , void * ( * )(void * ), void * ) = NULL;
static int( * real_munmap)(void *, size_t) = NULL;
static void( * real_pthread_exit)(void *) = NULL;
static void( * real_free)(void *) = NULL;
static void * ( * real_malloc)(size_t) = NULL;
static void * ( * real_realloc)(void *, size_t) = NULL;
static void * ( * real_mmap)(void *, size_t, int, int, int, off_t) = NULL;
static void * ( * real_dlopen)(const char * , int) = NULL;


/* Constructor */
static void __attribute__((constructor)) init(void) {
    // Set function pointers
    real_pthread_create = reinterpret_cast < decltype(real_pthread_create) > (dlsym(RTLD_NEXT, "pthread_create"));
    real_pthread_exit = reinterpret_cast < decltype(real_pthread_exit) > (dlsym(RTLD_NEXT, "pthread_exit"));
    real_malloc = reinterpret_cast < decltype(real_malloc) > (dlsym(RTLD_NEXT, "malloc"));
    real_realloc = reinterpret_cast < decltype(real_realloc) > (dlsym(RTLD_NEXT, "realloc"));
    real_free = reinterpret_cast < decltype(real_free) > (dlsym(RTLD_NEXT, "free"));
    real_mmap = reinterpret_cast < decltype(real_mmap) > (dlsym(RTLD_NEXT, "mmap"));
    real_munmap = reinterpret_cast < decltype(real_munmap) > (dlsym(RTLD_NEXT, "munmap"));
    real_dlopen = reinterpret_cast < decltype(real_dlopen) > (dlsym(RTLD_NEXT, "dlopen"));

    // Initialize isolation
    if (erim_init(8192, ERIM_FLAG_ISOLATE_TRUSTED)) {
        exit(EXIT_FAILURE);
    }
}


/* Auxiliary functions */
void setApplicationPermissions(const char* appID, int protectionFlag, int pkey) {
    auto it = apps.find(appID);
    if (it == apps.end()) {
        errExit("Application ID not found in the memory map.");
    }

    const std::vector<MemoryRegion>& memoryRegions = it->second;
    for (const MemoryRegion& region : memoryRegions) {
        if (pkey_mprotect(region.address, region.size, protectionFlag, pkey) == -1) {
            errExit("pkey_mprotect error");
        }
    }
}

std::string extractBaseName(const std::string& filePath) {
    size_t lastSlashPos = filePath.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        return filePath.substr(lastSlashPos + 1);
    }
    return filePath;
}

void getMemoryRegions(LibraryInfo *info) {
    const char* libraryPath = info->path;
    const char* appID = info->appID;

    std::ifstream mapsFile("/proc/self/maps");
    if (!mapsFile) {
        errExit("Failed to open /proc/self/maps");
    }

    std::string libraryName = extractBaseName(libraryPath);    
    
    std::string line;
    while (std::getline(mapsFile, line)) {
        if (line.find(libraryName) == std::string::npos) 
            continue;
            
        std::istringstream iss(line);
        std::string addressRange;

        if (!(iss >> addressRange))
            continue;

        std::istringstream rangeStream(addressRange);
        std::string startAddress, endAddress;
        std::getline(rangeStream, startAddress, '-');
        std::getline(rangeStream, endAddress);

        MemoryRegion memoryRegion;
        std::istringstream startStream(startAddress);
        startStream >> std::hex >> memoryRegion.address;

        std::istringstream endStream(endAddress);
        size_t start = (size_t) memoryRegion.address;
        endStream >> std::hex >> memoryRegion.size;

        memoryRegion.size -= start;
        apps[appID].push_back(memoryRegion);
    }

    mapsFile.close();
}

void printApps() {
    for (const auto& entry : apps) {
        const std::string& appID = entry.first;
        const std::vector<MemoryRegion>& memoryRegions = entry.second;

        std::cout << "App ID: " << appID << std::endl;

        for (const MemoryRegion& region : memoryRegions) {
            std::cout << "\tStart Address: " << region.address << ", Size: " << region.size << " bytes" << std::endl;
        }

        std::cout << std::endl;
    }
}

LibraryInfo parse_input(const char* input) {
    std::stringstream ss(input);
    std::string token1, token2;

    std::getline(ss, token1, ':');
    std::getline(ss, token2, ':');

    return { token1.c_str(), token2.c_str() };
}


/* Memory allocation and mapping */
void * malloc(size_t size) {
    void *ret;

    if (no_hook) {
        return (*real_malloc)(size);
    }

    no_hook = 1;
    ret = (*erim_malloc)(size);
    no_hook = 0;

    return ret;
}

void * realloc(void * ptr, size_t size) {
    void *ret;

    if (no_hook) {
        return (*real_realloc)(ptr, size);
    }

    no_hook = 1;
    ret = (*erim_realloc)(ptr, size);
    no_hook = 0;

    return ret;
}

void * mmap(void * addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void *ret;

    if (no_hook) {
        return (*real_mmap)(addr, length, prot, flags, fd, offset);
    }

    no_hook = 1;
    ret = erim_mmap_isolated(addr, length, prot, flags, fd, offset);
    no_hook = 0;

    return ret;
}

void free(void * ptr) {
    if (no_hook) {
        real_free(ptr);
        return;
    }

    no_hook = 1;
    erim_free(ptr);
    no_hook = 0;
}

int munmap(void * addr, size_t length) {
    int ret;

    if (no_hook) {
        return real_munmap(addr, length);
    }

    no_hook = 1;
    ret = erim_munmap(addr, length);
    no_hook = 0;

    return ret;
}


/* Library loading */
void * dlopen(const char * input, int flag) {
    //LibraryInfo info = parse_input(input);

    LibraryInfo info = {
        "application_id",
        input
    };

    void * handle = real_dlopen((&info)->path, flag);

    getMemoryRegions(&info);
    printApps();
    
    return handle;
}


/* Threads */
int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * ( * start_routine)(void * ), void * arg) {
    int result = real_pthread_create(thread, attr, start_routine, arg);

    if (result == 0) {
        int domain = ERIM_EXEC_DOMAIN(__rdpkru());
        {
            std::lock_guard<std::mutex> lock(runningThreadsMutex);
            runningThreads[domain].push_back(*thread);
        }
    }

    return result;
}

void pthread_exit(void* value_ptr) {
    pthread_t currentThread = pthread_self();
    int domain = ERIM_EXEC_DOMAIN(__rdpkru());

    {
        std::lock_guard<std::mutex> lock(runningThreadsMutex);
        std::vector<pthread_t> tvec = runningThreads[domain];
        tvec.erase(std::remove(tvec.begin(), tvec.end(), currentThread), tvec.end());
    }

    real_pthread_exit(value_ptr);
}
