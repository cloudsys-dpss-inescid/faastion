// Domain IDs from 0 to 15. Domain 0 should not be used (domain 0 uses musl's malloc).
#define DOMAINS 16

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

// PKRU to Domain conversion table.
#define PKRU_TO_DOMAIN(pkru) (\
    (pkru == 0x00000000) ? 0 : \
    (pkru == 0x55555551) ? 1 : \
    (pkru == 0x55555545) ? 2 : \
    (pkru == 0x55555515) ? 3 : \
    (pkru == 0x55555455) ? 4 : \
    (pkru == 0x55555155) ? 5 : \
    (pkru == 0x55554555) ? 6 : \
    (pkru == 0x55551555) ? 7 : \
    (pkru == 0x55545555) ? 8 : \
    (pkru == 0x55515555) ? 9 : \
    (pkru == 0x55455555) ? 10 : \
    (pkru == 0x55155555) ? 11 : \
    (pkru == 0x54555555) ? 12 : \
    (pkru == 0x51555555) ? 13 : \
    (pkru == 0x45555555) ? 14 : \
    (pkru == 0x15555555) ? 15 : \
    -1 \
)

#define DOMAIN_STACK_LOC(domain) ((void*)(11ull<<(44 - domain)))

#define read_stackptr(ptr) \
  do { \
    __asm__ volatile("movq %%rsp, %0" : "+m" (ptr)); \
  } while(0)

#define write_stackptr(ptr)					\
  do {									\
    __asm__ volatile("movq %0, %%rsp\n" : "=m" (ptr));	\
  } while(0)

#define switch_stack(newstackloc, oldstackloc)					\
  do {									\
    read_stackptr(oldstackloc);					\
    char * newstack = (char*) newstackloc;       \
    memcpy(newstack, oldstackloc, 1024);		\
    write_stackptr(newstack);	\
  } while(0)

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