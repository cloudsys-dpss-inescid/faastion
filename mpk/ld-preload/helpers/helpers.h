#include "../utils/appmap.h"

/*
 * Debug prints
 */
#ifdef PRELOAD_DBG
  #define PRELOAD_DBM(...)				\
    do {					\
      fprintf(stderr, __VA_ARGS__);		\
      fprintf(stderr, "\n");			\
    } while(0)
#else // disable debug
   #define PRELOAD_DBM(...)
#endif

char* extractBaseName(const char* filePath);
void getMemoryRegions(AppMap* map, char* id, const char* path);