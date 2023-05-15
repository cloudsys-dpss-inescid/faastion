#include "preload.h"


/* Function pointers declarations */
static int( * real_pthread_create)(pthread_t * , const pthread_attr_t * , void * ( * )(void * ), void * ) = NULL;
static void * ( * real_dlopen)(const char * , int) = NULL;

/* Constructor */
static void __attribute__((constructor)) init(void) {
    // Set function pointers
    real_pthread_create = reinterpret_cast < decltype(real_pthread_create) > (dlsym(RTLD_NEXT, "pthread_create"));
    real_dlopen = reinterpret_cast < decltype(real_dlopen) > (dlsym(RTLD_NEXT, "dlopen"));

    // Initialize isolation
    if (erim_init(8192, ERIM_FLAG_ISOLATE_TRUSTED | ERIM_FLAG_INTEGRITY_ONLY)) {
        exit(EXIT_FAILURE);
    }
}

/* Auxiliary functions */
void insert_item(const char * app_id, void * start_addr, size_t size) {
    // Create a new tuple with the start_addr and size
    auto new_entry = std::make_tuple(start_addr, size);

    // Check if the lib_name already exists in the map
    auto it = apps.find(app_id);
    if (it == apps.end()) {
        // If the lib_name doesn't exist, insert a new entry with a new list
        std::list < std::tuple < void * , size_t >> new_list;
        new_list.push_back(new_entry);
        apps.insert(std::make_pair(app_id, new_list));
    } else {
        // If the lib_name exists, append the new tuple to the existing list
        it -> second.push_back(new_entry);
    }
}

int callback(struct dl_phdr_info* info, size_t size, void* data) {
    lib_info * callback_data = (lib_info * ) data;
    const char* lib_name = callback_data->lib_name;
    const char* app_id = callback_data->app_id;

    if (!strcmp(info -> dlpi_name, lib_name)) {
        // Iterate over the program headers of the shared object
        for (int i = 0; i < info->dlpi_phnum; i++) {
            // Check if the program header is of type PT_DYNAMIC
            if (info->dlpi_phdr[i].p_type == PT_DYNAMIC) {
                // Retrieve the address of the dynamic section
                ElfW(Dyn)* dyn = reinterpret_cast<ElfW(Dyn)*>(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);

                ElfW(Sym)* symtab = nullptr;
                const char* strtab = nullptr;

                // Iterate over the entries in the dynamic section
                for (ElfW(Dyn)* entry = dyn; entry->d_tag != DT_NULL; entry++) {
                    // Find the dynamic symbol table
                    if (entry->d_tag == DT_SYMTAB) {
                        // Retrieve the address of the dynamic symbol table
                        symtab = reinterpret_cast<ElfW(Sym)*>(info->dlpi_addr + entry->d_un.d_ptr);
                    }
                    // Find the string table containing symbol names
                    else if (entry->d_tag == DT_STRTAB) {
                        // Retrieve the address of the string table
                        strtab = reinterpret_cast<const char*>(info->dlpi_addr + entry->d_un.d_ptr);
                    }
                }

                // Iterate over symbols in the dynamic symbol table
                if (symtab && strtab) {
                    for (ElfW(Sym)* symbol = symtab; symbol->st_name; symbol++) {
                        void* address = (void*)(info->dlpi_addr + symbol->st_value);
                        size_t size = symbol->st_size;
                        insert_item(app_id, address, size);
                    }
                }
            }
        }
        return 1;
    }

    return 0;
}

lib_info getInfo(const char * str) {
    std::stringstream ss(str);
    std::string token1, token2;

    std::getline(ss, token1, ':');
    std::getline(ss, token2, ':');

    const char* app_id = token1.c_str();
    const char* filename = token2.c_str();

    return {
        filename,
        app_id
    };
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
void * dlopen(const char * filename, int flag) {
    lib_info info = getInfo(filename);

    void * handle = real_dlopen((&info)->lib_name, flag);

    // get address and size of library
    dl_iterate_phdr(callback, & info);

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