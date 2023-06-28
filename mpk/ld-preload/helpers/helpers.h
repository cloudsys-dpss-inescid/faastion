#include "../utils/appmap.h"

void logMessage(const char* message, int verbose);
char* extractBaseName(const char* filePath);
void getMemoryRegions(AppMap* map, char* id, const char* path);