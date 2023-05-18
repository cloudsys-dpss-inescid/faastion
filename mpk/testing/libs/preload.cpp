#include "preload.h"

#define ERIM_FLAGS = ERIM_FLAG_ISOLATE_TRUSTED | ERIM_FLAG_INTEGRITY_ONLY

std::unordered_map<std::string, std::vector<MemoryRegion>> apps;


/* Function pointers declarations */
static int( * real_pthread_create)(pthread_t * , const pthread_attr_t * , void * ( * )(void * ), void * ) = NULL;
static void * ( * real_dlopen)(const char * , int) = NULL;


/* Constructor */
static void __attribute__((constructor)) init(void) {
    // Set function pointers
    real_pthread_create = reinterpret_cast < decltype(real_pthread_create) > (dlsym(RTLD_NEXT, "pthread_create"));
    real_dlopen = reinterpret_cast < decltype(real_dlopen) > (dlsym(RTLD_NEXT, "dlopen"));

    // Initialize isolation
    if (erim_init(8192, ERIM_FLAGS)) {
        exit(EXIT_FAILURE);
    }
}


/* Auxiliary functions */
void setApplicationPermissions(const char* appID, int protectionFlag) {
    auto it = apps.find(appID);
    if (it == apps.end()) {
        errExit("Application ID not found in the memory map.");
    }

    const std::vector<MemoryRegion>& memoryRegions = it->second;
    for (const MemoryRegion& region : memoryRegions) {
        if (pkey_mprotect(region.address, region.size, protectionFlag, ERIM_TRUSTED_DOMAIN_ID(ERIM_FLAGS)) == -1) {
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
    return erim_malloc(size);
}

void * zalloc(size_t size) {
    return erim_zalloc(size);
}

void * realloc(void * ptr, size_t size) {
    return erim_realloc(ptr, size);
}

void * mmap(void * addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return erim_mmap_isolated(addr, length, prot, flags, fd, offset);
}

void free(void * ptr) {
    erim_free(ptr);
}

int munmap(void * addr, size_t length) {
    return erim_munmap(addr, length);
}


/* Library loading */
void * dlopen(const char * input, int flag) {
    //LibraryInfo info = parse_input(input);

    LibraryInfo info = { // For testing
        "application_id",
        input
    };

    void * handle = real_dlopen((&info)->path, flag);

    getMemoryRegions(&info);
    //printApps();
    
    // Scanmem for wrpkru
    if (erim_memScan(NULL, NULL, ERIM_UNTRUSTED_PKRU)) {
        exit(EXIT_FAILURE);
    }
    return handle;
}


/* Threads */
int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * ( * start_routine)(void * ), void * arg) {
    //fprintf(stderr, "pthread_create(): thread with id %lu\n", *thread);
    return real_pthread_create(thread, attr, start_routine, arg);
}