#ifndef __PKRU_H__
#define __PKRU_H__

// Domain IDs from 0 to 15.
#define DOMAINS 16
#define DEFAULT_DOMAIN 0
#define LOADER_DOMAIN 1

#define RO_LOADER_DOMAIN 0x55555559

// TODO - have a table for constant conversion.
// Domain to PKRU conversion table.
static inline int DOMAIN_TO_PKRU(int domain) {
  switch (domain) {
    case 0:   return 0x0;
    case 1:   return 0x55555551;
    case 2:   return 0x5555554d;
    case 3:   return 0x5555551d;
    case 4:   return 0x5555545d;
    case 5:   return 0x5555515d;
    case 6:   return 0x5555455d;
    case 7:   return 0x5555155d;
    case 8:   return 0x5554555d;
    case 9:   return 0x5551555d;
    case 10:  return 0x5545555d;
    case 11:  return 0x5515555d;
    case 12:  return 0x5455555d;
    case 13:  return 0x5155555d;
    case 14:  return 0x4555555d;
    case 15:  return 0x1555555d;
    default:  return -1;
  }
}

// #define NO_ISOLATION 1

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