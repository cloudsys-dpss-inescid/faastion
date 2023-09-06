/*
 * test_application.c
 *
 */

#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Erim includes
#include <common.h>
#include <erim.h>

static __thread char* regular = NULL;

typedef struct {
    int a;
    int b;
} ThreadArgs;

void protectMemoryRegions(const char * library, int pkey) {
    FILE* mapsFile = fopen("/proc/self/maps", "r");
    if (!mapsFile) {
        fprintf(stderr, "Failed to open /proc/self/maps\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), mapsFile)) {
        if (strstr(line, library) == NULL) {
            continue;
        }

        unsigned long startAddress, endAddress;
        sscanf(line, "%lx-%lx", &startAddress, &endAddress);

        void * address = (void*)startAddress;
        size_t size = endAddress - startAddress;

        pkey_mprotect(address, size, PROT_READ|PROT_WRITE|PROT_EXEC, pkey);
    }

    fclose(mapsFile);
}

int incWrapper(int a) {
    int ret = 123;

    void *handle = dlopen("./libinc.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*inc)(int) = (int (*)(int))dlsym(handle, "inc");
    if (!inc) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libinc.so", 1);
    
    __wrpkru(ERIM_DOMAIN(1));
    ret = inc(a);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* incMain(void* arg) {
    int value = *(int*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(1), regular);
    value = incWrapper(value);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "inc = %d\n", value);

    return NULL;
}

void logWrapper(const char * message) {
    void *handle = dlopen("./liblog.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return;
    }

    void (*logMessage)(const char*) = (void (*)(const char*))dlsym(handle, "logMessage");
    if (!logMessage) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    protectMemoryRegions("liblog.so", 2);
    
    __wrpkru(ERIM_DOMAIN(2));
    logMessage(message);
    __wrpkru(ERIM_DOMAIN(0));
    
    dlclose(handle);
}

void* logMain(void* arg) {
    const char* message = (const char*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(2), regular);

    int size = strlen(message) + 1;
    memcpy(ERIM_DOMAIN_STACK_LOC(2), message, size);
    char *charPtr = (char*)ERIM_DOMAIN_STACK_LOC(2);
    char cpMessage[size];
    strcpy(cpMessage, charPtr);
    logWrapper(cpMessage);

    ERIM_SWITCH_BACK(regular);

    return NULL;
}


int negateWrapper(int a) {
    int ret = 123;

    void *handle = dlopen("./libnegate.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*negate)(int) = (int (*)(int))dlsym(handle, "negate");
    if (!negate) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libnegate.so", 11);
    
    __wrpkru(ERIM_DOMAIN(11));
    ret = negate(a);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* negateMain(void* arg) {
    int value = *(int*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(11), regular);
    value = negateWrapper(value);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "negate = %d\n", value);

    return NULL;
}

int negativeWrapper(int a) {
    int ret = 123;

    void *handle = dlopen("./libnegative.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*isNegative)(int) = (int (*)(int))dlsym(handle, "isNegative");
    if (!isNegative) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libnegative.so", 12);
    
    __wrpkru(ERIM_DOMAIN(12));
    ret = isNegative(a);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* negativeMain(void* arg) {
    int value = *(int*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(12), regular);
    value = negativeWrapper(value);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "negative = %d\n", value);

    return NULL;
}

int positiveWrapper(int a) {
    int ret = 123;

    void *handle = dlopen("./libpositive.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*isPositive)(int) = (int (*)(int))dlsym(handle, "isPositive");
    if (!isPositive) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libpositive.so", 13);
    
    __wrpkru(ERIM_DOMAIN(13));
    ret = isPositive(a);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* positiveMain(void* arg) {
    int value = *(int*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(13), regular);
    value = positiveWrapper(value);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "positive = %d\n", value);

    return NULL;
}

double squareWrapper(double c) {
    double ret = 123;

    void *handle = dlopen("./libsquare.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    double (*square)(double) = (double (*)(double))dlsym(handle, "square");
    if (!square) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libsquare.so", 14);
    
    __wrpkru(ERIM_DOMAIN(14));
    ret = square(c);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* squareMain(void* arg) {
    double value = *(double*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(14), regular);
    value = squareWrapper(value);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "square = %f\n", value);

    return NULL;
}

int absWrapper(int a) {
    int ret = 123;

    void *handle = dlopen("./libabs.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*absoluteValue)(int) = (int (*)(int))dlsym(handle, "absoluteValue");
    if (!absoluteValue) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libabs.so", 3);
    
    __wrpkru(ERIM_DOMAIN(3));
    ret = absoluteValue(a);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* absMain(void* arg) {
    int value = *(int*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(3), regular);
    value = absWrapper(value);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "abs = %d\n", value);

    return NULL;
}

int evenWrapper(int a) {
    int ret = 123;

    void *handle = dlopen("./libeven.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*isEven)(int) = (int (*)(int))dlsym(handle, "isEven");
    if (!isEven) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libeven.so", 5);
    
    __wrpkru(ERIM_DOMAIN(5));
    ret = isEven(a);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* evenMain(void* arg) {
    int value = *(int*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(5), regular);
    value = evenWrapper(value);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "even = %d\n", value);

    return NULL;
}

void helloWrapper() {
    void *handle = dlopen("./libhello.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return;
    }  

    void (*sayHello)() = (void (*)())dlsym(handle, "sayHello");
    if (!sayHello) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    protectMemoryRegions("libhello.so", 7);
    
    __wrpkru(ERIM_DOMAIN(7));
    sayHello();
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return;
}

void* helloMain() {
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(7), regular);
    helloWrapper();
    ERIM_SWITCH_BACK(regular);

    return NULL;
}

int addWrapper(int a, int b) {
    int ret = 123;

    void *handle = dlopen("./libadd.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*add)(int, int) = (int (*)(int, int))dlsym(handle, "add");
    if (!add) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libadd.so", 4);
    
    __wrpkru(ERIM_DOMAIN(4));
    ret = add(a, b);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* addMain(void* arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(4), regular);
    int value = addWrapper(args->a, args->b);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "add = %d\n", value);

    return NULL;
}

int subtractWrapper(int a, int b) {
    int ret = 123;

    void *handle = dlopen("./libsubtract.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*subtract)(int, int) = (int (*)(int, int))dlsym(handle, "subtract");
    if (!subtract) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libsubtract.so", 15);
    
    __wrpkru(ERIM_DOMAIN(15));
    ret = subtract(a, b);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* subtractMain(void* arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(15), regular);
    int value = subtractWrapper(args->a, args->b);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "subtract = %d\n", value);

    return NULL;
}

int multiplyWrapper(int a, int b) {
    int ret = 123;

    void *handle = dlopen("./libmultiply.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*multiply)(int, int) = (int (*)(int, int))dlsym(handle, "multiply");
    if (!multiply) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libmultiply.so", 10);
    
    __wrpkru(ERIM_DOMAIN(10));
    ret = multiply(a, b);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* multiplyMain(void* arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(10), regular);
    int value = multiplyWrapper(args->a, args->b);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "multiply = %d\n", value);

    return NULL;
}

int minWrapper(int a, int b) {
    int ret = 123;

    void *handle = dlopen("./libmin.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*min)(int, int) = (int (*)(int, int))dlsym(handle, "min");
    if (!min) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libmin.so", 9);
    
    __wrpkru(ERIM_DOMAIN(9));
    ret = min(a, b);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* minMain(void* arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(9), regular);
    int value = minWrapper(args->a, args->b);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "min = %d\n", value);

    return NULL;
}

int maxWrapper(int a, int b) {
    int ret = 123;

    void *handle = dlopen("./libmax.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*max)(int, int) = (int (*)(int, int))dlsym(handle, "max");
    if (!max) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libmax.so", 8);
    
    __wrpkru(ERIM_DOMAIN(8));
    ret = max(a, b);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

void* maxMain(void* arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(8), regular);
    int value = maxWrapper(args->a, args->b);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "max = %d\n", value);

    return NULL;
}

char* capitalizeWrapper(char * message) {
    void *handle = dlopen("./libcapitalize.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return "";
    }

    char* (*capitalize)(char*) = (char* (*)(char*))dlsym(handle, "capitalize");
    if (!capitalize) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return "";
    }
    
    protectMemoryRegions("libcapitalize.so", 6);
    
    __wrpkru(ERIM_DOMAIN(6));
    char* ret = capitalize(message);
    __wrpkru(ERIM_DOMAIN(0));
    
    dlclose(handle);

    return ret;
}

void* capitalizeMain(void* arg) {
    const char* message = (const char*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(6), regular);

    int size = strlen(message) + 1;
    memcpy(ERIM_DOMAIN_STACK_LOC(6), message, size);
    char *charPtr = (char*)ERIM_DOMAIN_STACK_LOC(6);
    char cpMessage[size];
    strcpy(cpMessage, charPtr);
    message = capitalizeWrapper(cpMessage);
    ERIM_SWITCH_BACK(regular);

    fprintf(stderr, "capitalize = %s\n", message);
    
    return NULL;
}

void run_threads() {   
    pthread_t * workers = (pthread_t*) malloc(sizeof(pthread_t)*15);
    const char* message = "Hello, Threads!";
    int a = 321;
    double c = 10;
    
    ThreadArgs *myArgs = malloc(sizeof(ThreadArgs));
    myArgs->a = a;
    myArgs->b = 123;

    if (pthread_create(&workers[0], NULL, incMain, (void*)&a) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[1], NULL, logMain, (void*)message) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[2], NULL, absMain, (void*)&a) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[3], NULL, addMain, myArgs) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[4], NULL, evenMain, (void*)&a) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[5], NULL, capitalizeMain, (void*)message) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[6], NULL, helloMain, NULL) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[7], NULL, maxMain, myArgs) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[8], NULL, minMain, myArgs) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[9], NULL, multiplyMain, myArgs) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[10], NULL, negateMain, (void*)&a) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[11], NULL, negativeMain, (void*)&a) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[12], NULL, positiveMain, (void*)&a) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[13], NULL, squareMain, (void*)&c) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[14], NULL, subtractMain, myArgs) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    
    for(int i = 0; i < 15; i++) {
        if(pthread_join(workers[i], NULL)) {
            perror("Thread can't join\n");
            exit(1);
        }
    }
    free(workers);
}

int main(int argc, char **argv) {
    if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED | ERIM_FLAG_SWAP_STACK, 16)) {
        exit(EXIT_FAILURE);
    }

    run_threads();

    return 0;
}
