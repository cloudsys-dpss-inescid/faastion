#ifndef LIBC_CALLGATE_PKRU
#define LIBC_CALLGATE_PKRU

#include <stdlib.h>
#include <string.h>

// Domain IDs from 0 to 15.
#define DOMAINS 16
#define DEFAULT_DOMAIN 0
#define LOADER_DOMAIN 1

// TODO - have a table for constant conversion.
// Domain to PKRU conversion table.
#define DOMAIN_TO_PKRU(domain) (\
    (domain == 0) ? 0x0 : \
    (domain == 1) ? 0x55555551 : \
    (domain == 2) ? 0x55555545 : \
    (domain == 3) ? 0x55555515 : \
    (domain == 4) ? 0x55555455 : \
    (domain == 5) ? 0x55555155 : \
    (domain == 6) ? 0x55554555 : \
    (domain == 7) ? 0x55551555 : \
    (domain == 8) ? 0x55545555 : \
    (domain == 9) ? 0x55515555 : \
    (domain == 10) ? 0x55455555 : \
    (domain == 11) ? 0x55155555 : \
    (domain == 12) ? 0x54555555 : \
    (domain == 13) ? 0x51555555 : \
    (domain == 14) ? 0x45555555 : \
    (domain == 15) ? 0x15555555 : \
    -1 \
)

#ifndef __wrpkru
#define __wrpkru(PKRU_ARG)			    \
  do {									\
    asm volatile ("xor %%ecx, %%ecx\n\txor %%edx, %%edx\n\tmov %0,%%eax\n\t.byte 0x0f,0x01,0xef\n\t" \
	      : : "n" (PKRU_ARG)					\
	      :"eax", "ecx", "edx");			\
  } while (0)
#endif

#define __wrpkrumem(PKRU_ARG)			    \
  do {									\
    asm volatile ("xor %%ecx, %%ecx\n\txor %%edx, %%edx\n\tmov %0,%%eax\n\t.byte 0x0f,0x01,0xef\n\t" \
	      : : "m" (PKRU_ARG)					\
	      :"eax", "ecx", "edx");			\
  } while (0)

#ifndef __rdpkru
#define __rdpkru()                              \
  ({                                            \
    unsigned int eax, edx;                      \
    unsigned int ecx = 0;                       \
    unsigned int pkru;                          \
    asm volatile(".byte 0x0f,0x01,0xee\n\t"     \
                 : "=a" (eax), "=d" (edx)       \
                 : "c" (ecx));                  \
    pkru = eax;                                 \
    pkru;                                       \
  })
#endif

struct domain;
extern struct domain *domains[DOMAINS];

// Prepares pkrus, prepares domain arenas, among other initializations.
int pkru_sandbox_init();
// Calls a function 'fun' in domain 'domain'.
int pkru_sandbox_call(int domain, void** ret, size_t* ret_size, void (*fun)(void*, size_t, void**, size_t*), void* arg, size_t arg_size);
// Returns the domain currently associated to the thread with the specified tid.
int get_thread_domain(pid_t tid);
// Adds a thread with the specified tid to a particular domain.
void set_thread_domain(pid_t tid, int domain);
// Removes a thread with the specified tid to a particular domain.
void del_thread_domain(pid_t tid, int domain);
// Books an available domain for a thread. Zero is returned in case all domains are used.
int book_available_domain(pid_t tid);

int initialize_domains();
void *get_arena(int domain);

void protect_library(const char* library, int pkey);

#endif