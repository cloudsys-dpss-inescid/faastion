#include <string.h>
#include <ctype.h>

char* capitalize(char* str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
    return str;
}
