#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memisolation.h" // Note: needed for domain.
#include "mpk.h"
#include "JNIWrapper.h"

const char* JNICALL Faastion_GetStringUTFChars(JNIEnv* env, jstring string, jboolean* isCopy) {
    JNI_DBM("[JNI]: Received GetStringUTFChars call from domain %d.", domain);

    __wrpkru(0);
    const char *original = (*GetStringUTFChars)(env, string, isCopy);
    __wrpkrumem(DOMAIN_TO_PKRU(domain));

    /* malloc string in chosen domain */
    size_t length = strlen(original) + 1;
    char *cstring = (char *)malloc((length + 1) * sizeof(char)); // +1 for the null terminator

    __wrpkru(0);
    memcpy(cstring, original, length);
    __wrpkrumem(DOMAIN_TO_PKRU(domain));

    return (const char*)cstring;
}

void JNICALL Faastion_ReleaseStringUTFChars(JNIEnv *env, jstring string, const char* utf) {
    JNI_DBM("[JNI]: Received ReleaseStringUTFChars call from domain %d.", domain);
    __wrpkru(0);
    (*ReleaseStringUTFChars)(env, string, utf);
    __wrpkrumem(DOMAIN_TO_PKRU(domain));
}

void init_jni_wrapper(JNIEnv* env) {
    GetStringUTFChars = (*env)->GetStringUTFChars;
    ReleaseStringUTFChars = (*env)->ReleaseStringUTFChars;
    (*(struct JNINativeInterface_ **)env)->GetStringUTFChars = Faastion_GetStringUTFChars;
    (*(struct JNINativeInterface_ **)env)->ReleaseStringUTFChars = Faastion_ReleaseStringUTFChars;
}
