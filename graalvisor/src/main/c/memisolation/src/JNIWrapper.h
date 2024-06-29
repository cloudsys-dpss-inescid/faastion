#ifndef JNI_WRAPPER_H
#define JNI_WRAPPER_H

#include <jni.h>

/*
 * Debug prints
 */
#ifdef JNI_DBG
  #define JNI_DBM(...)				\
    do {					\
      fprintf(stderr, __VA_ARGS__);		\
      fprintf(stderr, "\n");			\
    } while(0)
#else // disable debug
  #define JNI_DBM(...)
#endif

extern const char* (JNICALL *GetStringUTFChars)(JNIEnv *env, jstring str, jboolean *isCopy);
extern void (JNICALL *ReleaseStringUTFChars)(JNIEnv *env, jstring str, const char* chars);

const char* JNICALL Faastion_GetStringUTFChars(JNIEnv* env, jstring string, jboolean* isCopy);
void JNICALL Faastion_ReleaseStringUTFChars(JNIEnv *env, jstring string, const char* utf);

void init_jni_wrapper(JNIEnv* env);

#endif
