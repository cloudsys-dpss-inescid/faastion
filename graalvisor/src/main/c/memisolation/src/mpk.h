#ifndef MPK_H
#define MPK_H

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

#endif