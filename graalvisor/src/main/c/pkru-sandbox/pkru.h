#ifndef __PKRU_H__
#define __PKRU_H__

// Domain IDs from 0 to 15.
#define DOMAINS 16
#define DEFAULT_DOMAIN 0
#define LOADER_DOMAIN 1

// TODO - have a table for constant conversion.
// Domain to PKRU conversion table.
static inline int DOMAIN_TO_PKRU(int domain) {
  switch (domain) {
    case 0:   return 0x0;
    case 1:   return 0x55555551;
    case 2:   return 0x55555545;
    case 3:   return 0x55555515;
    case 4:   return 0x55555455;
    case 5:   return 0x55555155;
    case 6:   return 0x55554555;
    case 7:   return 0x55551555;
    case 8:   return 0x55545555;
    case 9:   return 0x55515555;
    case 10:  return 0x55455555;
    case 11:  return 0x55155555;
    case 12:  return 0x54555555;
    case 13:  return 0x51555555;
    case 14:  return 0x45555555;
    case 15:  return 0x15555555;
    default:  return -1;
  }
}

#ifdef NO_ISOLATION

#define __wrpkru(PKRU_ARG) {}

#define __wrpkrumem(PKRU_ARG) {}

#else

#define __wrpkru(PKRU_ARG)			    \
  do {									\
    asm volatile ("xor %%ecx, %%ecx\n\txor %%edx, %%edx\n\tmov %0,%%eax\n\t.byte 0x0f,0x01,0xef\n\t" \
	      : : "n" (PKRU_ARG)					\
	      :"eax", "ecx", "edx");			\
  } while (0)

#define __wrpkrumem(PKRU_ARG)			    \
  do {									\
    asm volatile ("xor %%ecx, %%ecx\n\txor %%edx, %%edx\n\tmov %0,%%eax\n\t.byte 0x0f,0x01,0xef\n\t" \
	      : : "m" (PKRU_ARG)					\
	      :"eax", "ecx", "edx");			\
  } while (0)

#endif /* NO_ISOLATION */

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

#endif // __PKRU_H__