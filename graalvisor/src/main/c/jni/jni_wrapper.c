#define _GNU_SOURCE

#include "pkru.h"
#include "hash_table.h"
#include "jni_wrapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#define DEFINE_WRAPPER1(ret, function, type, var) \
ret Wrapper##function(type var) {                                           \
    unsigned int privileged_pku;                                            \
    unsigned int pku;                                                       \
    unsigned int domain;                                                    \
    pku = __rdpkru();                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                           \
    privileged_pku = pku & 0x55555554;                                      \
    __wrpkrumem(privileged_pku);                                            \
    ret retval = (*(domainEnv[domain]))->function(domainEnv[domain]);       \
    __wrpkrumem(pku);                                                       \
    return retval;                                                          \
}

#define DEFINE_WRAPPER2(ret, function, type, var, type2, var2) \
ret Wrapper##function(type var, type2 var2) {                               \
    unsigned int privileged_pku;                                            \
    unsigned int pku;                                                       \
    unsigned int domain;                                                    \
    pku = __rdpkru();                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                           \
    privileged_pku = pku & 0x55555554;                                      \
    __wrpkrumem(privileged_pku);                                            \
    ret retval = (*(domainEnv[domain]))->function(domainEnv[domain], var2); \
    __wrpkrumem(pku);                                                       \
    return retval;                                                          \
}

#define DEFINE_WRAPPER3(ret, function, type, var, type2, var2, type3, var3) \
ret Wrapper##function(type var, type2 var2, type3 var3) {                           \
    unsigned int privileged_pku;                                                    \
    unsigned int pku;                                                               \
    unsigned int domain;                                                            \
    pku = __rdpkru();                                                               \
    domain = PKRU_TO_DOMAIN(pku);                                                   \
    privileged_pku = pku & 0x55555554;                                              \
    __wrpkrumem(privileged_pku);                                                    \
    ret retval = (*(domainEnv[domain]))->function(domainEnv[domain], var2, var3);   \
    __wrpkrumem(pku);                                                               \
    return retval;                                                                  \
}

#define DEFINE_WRAPPER4(ret, function, type, var, type2, var2, type3, var3, type4, var4) \
ret Wrapper##function(type var, type2 var2, type3 var3, type4 var4) {                   \
    unsigned int privileged_pku;                                                        \
    unsigned int pku;                                                                   \
    unsigned int domain;                                                                \
    pku = __rdpkru();                                                                   \
    domain = PKRU_TO_DOMAIN(pku);                                                       \
    privileged_pku = pku & 0x55555554;                                                  \
    __wrpkrumem(privileged_pku);                                                        \
    ret retval = (*(domainEnv[domain]))->function(domainEnv[domain], var2, var3, var4); \
    __wrpkrumem(pku);                                                                   \
    return retval;                                                                      \
}

#define DEFINE_WRAPPER5(ret, function, type, var, type2, var2, type3, var3, type4, var4, type5, var5)    \
ret Wrapper##function(type var, type2 var2, type3 var3, type4 var4, type5 var5) {                       \
    unsigned int privileged_pku;                                                                        \
    unsigned int pku;                                                                                   \
    unsigned int domain;                                                                                \
    pku = __rdpkru();                                                                                   \
    domain = PKRU_TO_DOMAIN(pku);                                                                       \
    privileged_pku = pku & 0x55555554;                                                                  \
    __wrpkrumem(privileged_pku);                                                                        \
    ret retval = (*(domainEnv[domain]))->function(domainEnv[domain], var2, var3, var4, var5);           \
    __wrpkrumem(pku);                                                                                   \
    return retval;                                                                                      \
}

#define DEFINE_VOID_WRAPPER1(function, type, var) \
void Wrapper##function(type var) {                                          \
    unsigned int privileged_pku;                                            \
    unsigned int pku;                                                       \
    unsigned int domain;                                                    \
    pku = __rdpkru();                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                           \
    privileged_pku = pku & 0x55555554;                                      \
    __wrpkrumem(privileged_pku);                                            \
    (*(domainEnv[domain]))->function(domainEnv[domain]);                    \
    __wrpkrumem(pku);                                                       \
}

#define DEFINE_VOID_WRAPPER2(function, type, var, type2, var2) \
void Wrapper##function(type var, type2 var2) {                              \
    unsigned int privileged_pku;                                            \
    unsigned int pku;                                                       \
    unsigned int domain;                                                    \
    pku = __rdpkru();                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                           \
    privileged_pku = pku & 0x55555554;                                      \
    __wrpkrumem(privileged_pku);                                            \
    (*(domainEnv[domain]))->function(domainEnv[domain], var2);              \
    __wrpkrumem(pku);                                                       \
}

#define DEFINE_VOID_WRAPPER3(function, type, var, type2, var2, type3, var3) \
void Wrapper##function(type var, type2 var2, type3 var3) {                  \
    unsigned int privileged_pku;                                            \
    unsigned int pku;                                                       \
    unsigned int domain;                                                    \
    pku = __rdpkru();                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                           \
    privileged_pku = pku & 0x55555554;                                      \
    __wrpkrumem(privileged_pku);                                            \
    (*(domainEnv[domain]))->function(domainEnv[domain], var2, var3);        \
    __wrpkrumem(pku);                                                       \
}

#define DEFINE_VOID_WRAPPER4(function, type, var, type2, var2, type3, var3, type4, var4) \
void Wrapper##function(type var, type2 var2, type3 var3, type4 var4) {                  \
    unsigned int privileged_pku;                                                        \
    unsigned int pku;                                                                   \
    unsigned int domain;                                                                \
    pku = __rdpkru();                                                                   \
    domain = PKRU_TO_DOMAIN(pku);                                                       \
    privileged_pku = pku & 0x55555554;                                                  \
    __wrpkrumem(privileged_pku);                                                        \
    (*(domainEnv[domain]))->function(domainEnv[domain], var2, var3, var4);              \
    __wrpkrumem(pku);                                                                   \
}

#define DEFINE_VOID_WRAPPER5(function, type, var, type2, var2, type3, var3, type4, var4, type5, var5)    \
void Wrapper##function(type var, type2 var2, type3 var3, type4 var4, type5 var5) {                      \
    unsigned int privileged_pku;                                                                        \
    unsigned int pku;                                                                                   \
    unsigned int domain;                                                                                \
    pku = __rdpkru();                                                                                   \
    domain = PKRU_TO_DOMAIN(pku);                                                                       \
    privileged_pku = pku & 0x55555554;                                                                  \
    __wrpkrumem(privileged_pku);                                                                        \
    (*(domainEnv[domain]))->function(domainEnv[domain], var2, var3, var4, var5);                        \
    __wrpkrumem(pku);                                                                                   \
}

#define DEFINE_VA_WRAPPER3(ret, function, type, var, type2, var2, type3, var3)   \
ret Wrapper##function(type var, type2 var2, type3 var3, ...) {                              \
    unsigned int privileged_pku;                                                            \
    unsigned int pku;                                                                       \
    unsigned int domain;                                                                    \
    pku = __rdpkru();                                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                                           \
    privileged_pku = pku & 0x55555554;                                                      \
    __wrpkrumem(privileged_pku);                                                            \
    va_list args;                                                                           \
    va_start(args, var3);                                                                   \
    ret retval = (*(domainEnv[domain]))->function##V(domainEnv[domain], var2, var3, args);  \
    va_end(args);                                                                           \
    __wrpkrumem(pku);                                                                       \
    return retval;                                                                          \
}

#define DEFINE_VA_WRAPPER4(ret, function, type, var, type2, var2, type3, var3, type4, var4)      \
ret Wrapper##function(type var, type2 var2, type3 var3, type4 var4, ...) {                          \
    unsigned int privileged_pku;                                                                    \
    unsigned int pku;                                                                               \
    unsigned int domain;                                                                            \
    pku = __rdpkru();                                                                               \
    domain = PKRU_TO_DOMAIN(pku);                                                                   \
    privileged_pku = pku & 0x55555554;                                                              \
    __wrpkrumem(privileged_pku);                                                                    \
    va_list args;                                                                                   \
    va_start(args, var4);                                                                           \
    ret retval = (*(domainEnv[domain]))->function##V(domainEnv[domain], var2, var3, var4, args);    \
    va_end(args);                                                                                   \
    __wrpkrumem(pku);                                                                               \
    return retval;                                                                                  \
}

#define DEFINE_VA_VOID_WRAPPER3(function, type, var, type2, var2, type3, var3)   \
void Wrapper##function(type var, type2 var2, type3 var3, ...) {                 \
    unsigned int privileged_pku;                                                \
    unsigned int pku;                                                           \
    unsigned int domain;                                                        \
    pku = __rdpkru();                                                           \
    domain = PKRU_TO_DOMAIN(pku);                                               \
    privileged_pku = pku & 0x55555554;                                          \
    __wrpkrumem(privileged_pku);                                                \
    va_list args;                                                               \
    va_start(args, var3);                                                       \
    (*(domainEnv[domain]))->function##V(domainEnv[domain], var2, var3, args);   \
    va_end(args);                                                               \
    __wrpkrumem(pku);                                                           \
}

#define DEFINE_VA_VOID_WRAPPER4(function, type, var, type2, var2, type3, var3, type4, var4)      \
void Wrapper##function(type var, type2 var2, type3 var3, type4 var4, ...) {                     \
    unsigned int privileged_pku;                                                                \
    unsigned int pku;                                                                           \
    unsigned int domain;                                                                        \
    pku = __rdpkru();                                                                           \
    domain = PKRU_TO_DOMAIN(pku);                                                               \
    privileged_pku = pku & 0x55555554;                                                          \
    __wrpkrumem(privileged_pku);                                                                \
    va_list args;                                                                               \
    va_start(args, var4);                                                                       \
    (*(domainEnv[domain]))->function##V(domainEnv[domain], var2, var3, var4, args);             \
    va_end(args);                                                                               \
    __wrpkrumem(pku);                                                                           \
}

#define DEFINE_ARRAY_WRAPPER3(ret, function, type, var, type2, var2, type3, isCopy, get_len) \
ret Wrapper##function(type var, type2 var2, type3 isCopy) {                                   \
    unsigned int privileged_pku;                                                            \
    unsigned int pku;                                                                       \
    unsigned int domain;                                                                    \
    pku = __rdpkru();                                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                                           \
    privileged_pku = pku & 0x55555554;                                                      \
    __wrpkrumem(privileged_pku);                                                            \
    jsize len = (*(domainEnv[domain]))->get_len(domainEnv[domain], var2);                   \
    ret addr = (*(domainEnv[domain]))->function(domainEnv[domain], var2, NULL);             \
    ret retval = convert_to_c(addr, len);                                                   \
    if (isCopy) *isCopy = JNI_TRUE;                                                         \
    __wrpkrumem(pku);                                                                       \
    return retval;                                                                          \
}

#define DEFINE_RELEASE_ARRAY_WRAPPER3(function, type, var, type2, var2, type3, c_addr) \
void Wrapper##function(type var, type2 var2, type3 c_addr) {                                \
    unsigned int privileged_pku;                                                            \
    unsigned int pku;                                                                       \
    unsigned int domain;                                                                    \
    pku = __rdpkru();                                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                                           \
    privileged_pku = pku & 0x55555554;                                                      \
    __wrpkrumem(privileged_pku);                                                            \
    type3 java_addr = release_c_array(c_addr, 0);                                           \
    (*(domainEnv[domain]))->function(domainEnv[domain], var2, java_addr);                   \
    __wrpkrumem(pku);                                                                       \
}

#define DEFINE_RELEASE_ARRAY_WRAPPER4(function, type, var, type2, var2, type3, c_addr, type4, mode) \
void Wrapper##function(type var, type2 var2, type3 c_addr, type4 mode) {                    \
    unsigned int privileged_pku;                                                            \
    unsigned int pku;                                                                       \
    unsigned int domain;                                                                    \
    pku = __rdpkru();                                                                       \
    domain = PKRU_TO_DOMAIN(pku);                                                           \
    privileged_pku = pku & 0x55555554;                                                      \
    __wrpkrumem(privileged_pku);                                                            \
    type3 java_addr = release_c_array(c_addr, mode);                                        \
    (*(domainEnv[domain]))->function(domainEnv[domain], var2, java_addr, mode);             \
    __wrpkrumem(pku);                                                                       \
}

#define DEFINE_UNSUPPORTED_WRAPPER2(ret, function, type, var, type2, var2) \
ret Wrapper##function(type var, type2 var2) {               \
    return NULL;                                            \
}

struct c_array {
    void *base;
    jsize len;
};

JNIEnv *domainEnv[DOMAINS] = {0};
struct hash_table *to_c;        // maps java addr (uintptr) to `c array` (struct c_array *) 
struct hash_table *to_java;     // maps c addr (uintptr) to java addr (uintptr)
JNIWrapper *globalWrapper; // Accessible in loader domain (1)

static inline unsigned int PKRU_TO_DOMAIN(unsigned int pkru) {
    pkru = pkru ^ 0x55555551;
    switch (pkru) {
    case 0:             return 1;
    case 0x10:          return 2;
    case 0x40:          return 3;
    case 0x100:         return 4;
    case 0x400:         return 5;
    case 0x1000:        return 6;
    case 0x4000:        return 7;
    case 0x10000:       return 8;
    case 0x40000:       return 9;
    case 0x100000:      return 10;
    case 0x400000:      return 11;
    case 0x1000000:     return 12;
    case 0x4000000:     return 13;
    case 0x10000000:    return 14;
    case 0x40000000:    return 15;
    default:            return 0;
    }
}

void *convert_to_c(const void *java_addr, jsize len) {
    struct c_array *c_addr;
    if ((c_addr = hash_table_lookup(to_c, (unsigned long)java_addr)) != NULL)
        return c_addr->base;

    c_addr = malloc(sizeof(struct c_array));
    c_addr->base = malloc(len);    
    c_addr->len = len;
    memcpy(c_addr->base, java_addr, len);
    hash_table_insert(to_c, (unsigned long)java_addr, (void *)c_addr);
    hash_table_insert(to_java, (unsigned long)c_addr->base, (void *)java_addr);

    return c_addr->base;
}

/*
 * The mode argument provides information on how the array buffer should be released
 * mode 0           : copy back the content and free the elems buffer
 * mode JNI_COMMIT  : copy back the content but do not free the elems buffer
 * mode JNI_ABORT   : free the buffer without copying back the possible changes
 */
void *release_c_array(const void *base, jint mode) {
    void *java_addr = hash_table_lookup(to_java, (unsigned long)base);
    if (java_addr == NULL) {
        fprintf(stderr, "JNIWrapper: could not convert address to java\n");
        goto out;
    }

    struct c_array *c_addr;
    c_addr = hash_table_lookup(to_c, (unsigned long)java_addr);

    if (mode == JNI_ABORT)
        goto abort;

    memcpy(java_addr, base, c_addr->len);
    if (mode == JNI_COMMIT)
        goto out;

abort:
    free(c_addr->base);
    free(c_addr);
    hash_table_remove(to_c, (unsigned long)java_addr, NULL);
    hash_table_remove(to_java, (unsigned long)base, NULL);

out:
    return java_addr;
}

DEFINE_WRAPPER1(jint, GetVersion, JNIEnv *, env)
DEFINE_WRAPPER5(jclass, DefineClass, JNIEnv *, env, const char *, name, jobject, loader, const jbyte *, buf, jsize, len)
DEFINE_WRAPPER2(jclass, FindClass, JNIEnv*, env, const char *, name)
DEFINE_WRAPPER2(jmethodID, FromReflectedMethod, JNIEnv *, env, jobject, method)
DEFINE_WRAPPER2(jfieldID, FromReflectedField, JNIEnv *, env, jobject, field)
DEFINE_WRAPPER4(jobject, ToReflectedMethod, JNIEnv *, env, jclass, cls, jmethodID, methodID, jboolean, isStatic)
DEFINE_WRAPPER2(jclass, GetSuperclass, JNIEnv *, env, jclass, sub)
DEFINE_WRAPPER3(jboolean, IsAssignableFrom, JNIEnv *, env, jclass, sub, jclass, sup)
DEFINE_WRAPPER4(jobject, ToReflectedField, JNIEnv *, env, jclass, cls, jfieldID, fieldID, jboolean, isStatic)
DEFINE_WRAPPER2(jint, Throw, JNIEnv *, env, jthrowable, obj)
DEFINE_WRAPPER3(jint, ThrowNew, JNIEnv *, env, jclass, clazz, const char *, msg)
DEFINE_WRAPPER1(jthrowable, ExceptionOccurred, JNIEnv *, env)
DEFINE_VOID_WRAPPER1(ExceptionDescribe, JNIEnv *, env)
DEFINE_VOID_WRAPPER1(ExceptionClear, JNIEnv *, env)
DEFINE_VOID_WRAPPER2(FatalError, JNIEnv *, env, const char *, msg)
DEFINE_WRAPPER2(jint, PushLocalFrame, JNIEnv *, env, jint, capacity)
DEFINE_WRAPPER2(jobject, PopLocalFrame, JNIEnv *, env, jobject, result)
DEFINE_WRAPPER2(jobject, NewGlobalRef, JNIEnv *, env, jobject, lobj)
DEFINE_VOID_WRAPPER2(DeleteGlobalRef, JNIEnv *, env, jobject, gref)
DEFINE_VOID_WRAPPER2(DeleteLocalRef, JNIEnv *, env, jobject, obj)
DEFINE_WRAPPER3(jboolean, IsSameObject, JNIEnv *, env, jobject, obj1, jobject, obj2)
DEFINE_WRAPPER2(jobject, NewLocalRef, JNIEnv *, env, jobject, ref)
DEFINE_WRAPPER2(jint, EnsureLocalCapacity, JNIEnv *, env, jint, capacity)
DEFINE_WRAPPER2(jobject, AllocObject, JNIEnv *, env, jclass, clazz)
DEFINE_VA_WRAPPER3(jobject, NewObject, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jobject, NewObjectV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jobject, NewObjectA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_WRAPPER2(jclass, GetObjectClass, JNIEnv *, env, jobject, obj)
DEFINE_WRAPPER3(jboolean, IsInstanceOf, JNIEnv *, env, jobject, obj, jclass, clazz)
DEFINE_WRAPPER4(jmethodID, GetMethodID, JNIEnv *, env, jclass, clazz, const char *, name, const char *, sig)
DEFINE_VA_WRAPPER3(jobject, CallObjectMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jobject, CallObjectMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jobject, CallObjectMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jboolean, CallBooleanMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jboolean, CallBooleanMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jboolean, CallBooleanMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jbyte, CallByteMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jbyte, CallByteMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jbyte, CallByteMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jchar, CallCharMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jchar, CallCharMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jchar, CallCharMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jshort, CallShortMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jshort, CallShortMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jshort, CallShortMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jint, CallIntMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jint, CallIntMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jint, CallIntMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jlong, CallLongMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jlong, CallLongMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jlong, CallLongMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jfloat, CallFloatMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jfloat, CallFloatMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jfloat, CallFloatMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jdouble, CallDoubleMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_WRAPPER4(jdouble, CallDoubleMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jdouble, CallDoubleMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_VOID_WRAPPER3(CallVoidMethod, JNIEnv *, env, jobject, obj, jmethodID, methodID)
DEFINE_VOID_WRAPPER4(CallVoidMethodV, JNIEnv *, env, jobject, obj, jmethodID, methodID, va_list, args)
DEFINE_VOID_WRAPPER4(CallVoidMethodA, JNIEnv *, env, jobject, obj, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jobject, CallNonvirtualObjectMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jobject, CallNonvirtualObjectMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jobject, CallNonvirtualObjectMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jboolean, CallNonvirtualBooleanMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jboolean, CallNonvirtualBooleanMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jboolean, CallNonvirtualBooleanMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jbyte, CallNonvirtualByteMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jbyte, CallNonvirtualByteMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jbyte, CallNonvirtualByteMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jchar, CallNonvirtualCharMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jchar, CallNonvirtualCharMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jchar, CallNonvirtualCharMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jshort, CallNonvirtualShortMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jshort, CallNonvirtualShortMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jshort, CallNonvirtualShortMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jint, CallNonvirtualIntMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jint, CallNonvirtualIntMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jint, CallNonvirtualIntMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jlong, CallNonvirtualLongMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jlong, CallNonvirtualLongMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jlong, CallNonvirtualLongMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jfloat, CallNonvirtualFloatMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jfloat, CallNonvirtualFloatMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jfloat, CallNonvirtualFloatMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER4(jdouble, CallNonvirtualDoubleMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER5(jdouble, CallNonvirtualDoubleMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER5(jdouble, CallNonvirtualDoubleMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_VOID_WRAPPER4(CallNonvirtualVoidMethod, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID)
DEFINE_VOID_WRAPPER5(CallNonvirtualVoidMethodV, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_VOID_WRAPPER5(CallNonvirtualVoidMethodA, JNIEnv *, env, jobject, obj, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_WRAPPER4(jfieldID, GetFieldID, JNIEnv *, env, jclass, clazz, const char *, name, const char *, sig)
DEFINE_WRAPPER3(jobject, GetObjectField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_WRAPPER3(jboolean, GetBooleanField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_WRAPPER3(jbyte, GetByteField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_WRAPPER3(jchar, GetCharField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_WRAPPER3(jshort, GetShortField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_WRAPPER3(jint, GetIntField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_WRAPPER3(jlong, GetLongField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_WRAPPER3(jfloat, GetFloatField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_WRAPPER3(jdouble, GetDoubleField, JNIEnv *, env, jobject, obj, jfieldID, fieldID)
DEFINE_VOID_WRAPPER4(SetObjectField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jobject, val)
DEFINE_VOID_WRAPPER4(SetBooleanField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jboolean, val)
DEFINE_VOID_WRAPPER4(SetByteField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jbyte, val)
DEFINE_VOID_WRAPPER4(SetCharField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jchar, val)
DEFINE_VOID_WRAPPER4(SetShortField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jshort, val)
DEFINE_VOID_WRAPPER4(SetIntField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jint, val)
DEFINE_VOID_WRAPPER4(SetLongField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jlong, val)
DEFINE_VOID_WRAPPER4(SetFloatField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jfloat, val)
DEFINE_VOID_WRAPPER4(SetDoubleField, JNIEnv *, env, jobject, obj, jfieldID, fieldID, jdouble, val)
DEFINE_WRAPPER4(jmethodID, GetStaticMethodID, JNIEnv *, env, jclass, clazz, const char *, name, const char *, sig)
DEFINE_VA_WRAPPER3(jobject, CallStaticObjectMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jobject, CallStaticObjectMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jobject, CallStaticObjectMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jboolean, CallStaticBooleanMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jboolean, CallStaticBooleanMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jboolean, CallStaticBooleanMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jbyte, CallStaticByteMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jbyte, CallStaticByteMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jbyte, CallStaticByteMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jchar, CallStaticCharMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jchar, CallStaticCharMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jchar, CallStaticCharMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jshort, CallStaticShortMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jshort, CallStaticShortMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jshort, CallStaticShortMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jint, CallStaticIntMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jint, CallStaticIntMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jint, CallStaticIntMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jlong, CallStaticLongMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jlong, CallStaticLongMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jlong, CallStaticLongMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jfloat, CallStaticFloatMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jfloat, CallStaticFloatMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jfloat, CallStaticFloatMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_WRAPPER3(jdouble, CallStaticDoubleMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_WRAPPER4(jdouble, CallStaticDoubleMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_WRAPPER4(jdouble, CallStaticDoubleMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_VA_VOID_WRAPPER3(CallStaticVoidMethod, JNIEnv *, env, jclass, clazz, jmethodID, methodID)
DEFINE_VOID_WRAPPER4(CallStaticVoidMethodV, JNIEnv *, env, jclass, clazz, jmethodID, methodID, va_list, args)
DEFINE_VOID_WRAPPER4(CallStaticVoidMethodA, JNIEnv *, env, jclass, clazz, jmethodID, methodID, jvalue *, args)
DEFINE_WRAPPER4(jfieldID, GetStaticFieldID, JNIEnv *, env, jclass, clazz, const char *, name, const char *, sig)
DEFINE_WRAPPER3(jobject, GetStaticObjectField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_WRAPPER3(jboolean, GetStaticBooleanField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_WRAPPER3(jbyte, GetStaticByteField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_WRAPPER3(jchar, GetStaticCharField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_WRAPPER3(jshort, GetStaticShortField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_WRAPPER3(jint, GetStaticIntField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_WRAPPER3(jlong, GetStaticLongField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_WRAPPER3(jfloat, GetStaticFloatField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_WRAPPER3(jdouble, GetStaticDoubleField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID)
DEFINE_VOID_WRAPPER4(SetStaticObjectField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jobject, val)
DEFINE_VOID_WRAPPER4(SetStaticBooleanField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jboolean, val)
DEFINE_VOID_WRAPPER4(SetStaticByteField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jbyte, val)
DEFINE_VOID_WRAPPER4(SetStaticCharField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jchar, val)
DEFINE_VOID_WRAPPER4(SetStaticShortField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jshort, val)
DEFINE_VOID_WRAPPER4(SetStaticIntField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jint, val)
DEFINE_VOID_WRAPPER4(SetStaticLongField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jlong, val)
DEFINE_VOID_WRAPPER4(SetStaticFloatField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jfloat, val)
DEFINE_VOID_WRAPPER4(SetStaticDoubleField, JNIEnv *, env, jclass, clazz, jfieldID, fieldID, jdouble, val)
DEFINE_WRAPPER3(jstring, NewString, JNIEnv *, env, const jchar *, unicode, jsize, len)
DEFINE_WRAPPER2(jsize, GetStringLength, JNIEnv *, env, jstring, str)
DEFINE_ARRAY_WRAPPER3(const jchar *, GetStringChars, JNIEnv *, env, jstring, str, jboolean *, isCopy, GetStringLength)
DEFINE_RELEASE_ARRAY_WRAPPER3(ReleaseStringChars, JNIEnv *, env, jstring, str, const jchar *, chars)
DEFINE_WRAPPER2(jstring, NewStringUTF, JNIEnv *, env, const char *, utf)
DEFINE_WRAPPER2(jsize, GetStringUTFLength, JNIEnv *, env, jstring, str)
DEFINE_ARRAY_WRAPPER3(const char *, GetStringUTFChars, JNIEnv *, env, jstring, str, jboolean *, isCopy, GetStringUTFLength)
DEFINE_RELEASE_ARRAY_WRAPPER3(ReleaseStringUTFChars, JNIEnv *, env, jstring, str, const char *, chars)
DEFINE_WRAPPER2(jsize, GetArrayLength, JNIEnv *, env, jarray, array)
DEFINE_WRAPPER4(jobjectArray, NewObjectArray, JNIEnv *, env, jsize, len, jclass, clazz, jobject, init)
DEFINE_WRAPPER3(jobject, GetObjectArrayElement, JNIEnv *, env, jobjectArray, array, jsize, index)
DEFINE_VOID_WRAPPER4(SetObjectArrayElement, JNIEnv *, env, jobjectArray, array, jsize, index, jobject, val)
DEFINE_WRAPPER2(jbooleanArray, NewBooleanArray, JNIEnv *, env, jsize, len)
DEFINE_WRAPPER2(jbyteArray, NewByteArray, JNIEnv *, env, jsize, len)
DEFINE_WRAPPER2(jcharArray, NewCharArray, JNIEnv *, env, jsize, len)
DEFINE_WRAPPER2(jshortArray, NewShortArray, JNIEnv *, env, jsize, len)
DEFINE_WRAPPER2(jintArray, NewIntArray, JNIEnv *, env, jsize, len)
DEFINE_WRAPPER2(jlongArray, NewLongArray, JNIEnv *, env, jsize, len)
DEFINE_WRAPPER2(jfloatArray, NewFloatArray, JNIEnv *, env, jsize, len)
DEFINE_WRAPPER2(jdoubleArray, NewDoubleArray, JNIEnv *, env, jsize, len)
DEFINE_ARRAY_WRAPPER3(jboolean *, GetBooleanArrayElements, JNIEnv *, env, jbooleanArray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_ARRAY_WRAPPER3(jbyte *, GetByteArrayElements, JNIEnv *, env, jbyteArray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_ARRAY_WRAPPER3(jchar *, GetCharArrayElements, JNIEnv *, env, jcharArray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_ARRAY_WRAPPER3(jshort *, GetShortArrayElements, JNIEnv *, env, jshortArray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_ARRAY_WRAPPER3(jint *, GetIntArrayElements, JNIEnv *, env, jintArray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_ARRAY_WRAPPER3(jlong *, GetLongArrayElements, JNIEnv *, env, jlongArray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_ARRAY_WRAPPER3(jfloat *, GetFloatArrayElements, JNIEnv *, env, jfloatArray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_ARRAY_WRAPPER3(jdouble *, GetDoubleArrayElements, JNIEnv *, env, jdoubleArray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleaseBooleanArrayElements, JNIEnv *, env, jbooleanArray, array, jboolean *, elems, jint, mode)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleaseByteArrayElements, JNIEnv *, env, jbyteArray, array, jbyte *, elems, jint, mode)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleaseCharArrayElements, JNIEnv *, env, jcharArray, array, jchar *, elems, jint, mode)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleaseShortArrayElements, JNIEnv *, env, jshortArray, array, jshort *, elems, jint, mode)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleaseIntArrayElements, JNIEnv *, env, jintArray, array, jint *, elems, jint, mode)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleaseLongArrayElements, JNIEnv *, env, jlongArray, array, jlong *, elems, jint, mode)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleaseFloatArrayElements, JNIEnv *, env, jfloatArray, array, jfloat *, elems, jint, mode)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleaseDoubleArrayElements, JNIEnv *, env, jdoubleArray, array, jdouble *, elems, jint, mode)
DEFINE_VOID_WRAPPER5(GetBooleanArrayRegion, JNIEnv *, env, jbooleanArray, array, jsize, start, jsize, l, jboolean *, buf)
DEFINE_VOID_WRAPPER5(GetByteArrayRegion, JNIEnv *, env, jbyteArray, array, jsize, start, jsize, l, jbyte *, buf)
DEFINE_VOID_WRAPPER5(GetCharArrayRegion, JNIEnv *, env, jcharArray, array, jsize, start, jsize, l, jchar *, buf)
DEFINE_VOID_WRAPPER5(GetShortArrayRegion, JNIEnv *, env, jshortArray, array, jsize, start, jsize, l, jshort *, buf)
DEFINE_VOID_WRAPPER5(GetIntArrayRegion, JNIEnv *, env, jintArray, array, jsize, start, jsize, l, jint *, buf)
DEFINE_VOID_WRAPPER5(GetLongArrayRegion, JNIEnv *, env, jlongArray, array, jsize, start, jsize, l, jlong *, buf)
DEFINE_VOID_WRAPPER5(GetFloatArrayRegion, JNIEnv *, env, jfloatArray, array, jsize, start, jsize, l, jfloat *, buf)
DEFINE_VOID_WRAPPER5(GetDoubleArrayRegion, JNIEnv *, env, jdoubleArray, array, jsize, start, jsize, l, jdouble *, buf)
DEFINE_VOID_WRAPPER5(SetBooleanArrayRegion, JNIEnv *, env, jbooleanArray, array, jsize, start, jsize, l, const jboolean *, buf)
DEFINE_VOID_WRAPPER5(SetByteArrayRegion, JNIEnv *, env, jbyteArray, array, jsize, start, jsize, l, const jbyte *, buf)
DEFINE_VOID_WRAPPER5(SetCharArrayRegion, JNIEnv *, env, jcharArray, array, jsize, start, jsize, l, const jchar *, buf)
DEFINE_VOID_WRAPPER5(SetShortArrayRegion, JNIEnv *, env, jshortArray, array, jsize, start, jsize, l, const jshort *, buf)
DEFINE_VOID_WRAPPER5(SetIntArrayRegion, JNIEnv *, env, jintArray, array, jsize, start, jsize, l, const jint *, buf)
DEFINE_VOID_WRAPPER5(SetLongArrayRegion, JNIEnv *, env, jlongArray, array, jsize, start, jsize, l, const jlong *, buf)
DEFINE_VOID_WRAPPER5(SetFloatArrayRegion, JNIEnv *, env, jfloatArray, array, jsize, start, jsize, l, const jfloat *, buf)
DEFINE_VOID_WRAPPER5(SetDoubleArrayRegion, JNIEnv *, env, jdoubleArray, array, jsize, start, jsize, l, const jdouble *, buf)
DEFINE_WRAPPER4(jint, RegisterNatives, JNIEnv *, env, jclass, clazz, const JNINativeMethod *, methods, jint, nMethods)
DEFINE_WRAPPER2(jint, UnregisterNatives, JNIEnv *, env, jclass, clazz)
DEFINE_WRAPPER2(jint, MonitorEnter, JNIEnv *, env, jobject, obj)
DEFINE_WRAPPER2(jint, MonitorExit, JNIEnv *, env, jobject, obj)
DEFINE_WRAPPER2(jint, GetJavaVM, JNIEnv *, env, JavaVM **, vm)
DEFINE_VOID_WRAPPER5(GetStringRegion, JNIEnv *, env, jstring, str, jsize, start, jsize, len, jchar *, buf)
DEFINE_VOID_WRAPPER5(GetStringUTFRegion, JNIEnv *, env, jstring, str, jsize, start, jsize, len, char *, buf)
DEFINE_ARRAY_WRAPPER3(void *, GetPrimitiveArrayCritical, JNIEnv *, env, jarray, array, jboolean *, isCopy, GetArrayLength)
DEFINE_RELEASE_ARRAY_WRAPPER4(ReleasePrimitiveArrayCritical, JNIEnv *, env, jarray, array, void *, carray, jint, mode)
DEFINE_ARRAY_WRAPPER3(const jchar *, GetStringCritical, JNIEnv *, env, jstring, string, jboolean *, isCopy, GetStringLength)
DEFINE_RELEASE_ARRAY_WRAPPER3(ReleaseStringCritical, JNIEnv *, env, jstring, string, const jchar *, cstring)
DEFINE_WRAPPER2(jweak, NewWeakGlobalRef, JNIEnv *, env, jobject, obj)
DEFINE_VOID_WRAPPER2(DeleteWeakGlobalRef, JNIEnv *, env, jweak, ref)
DEFINE_WRAPPER1(jboolean, ExceptionCheck, JNIEnv *, env)
DEFINE_WRAPPER3(jobject, NewDirectByteBuffer, JNIEnv *, env, void *, buf, jlong, len)
DEFINE_UNSUPPORTED_WRAPPER2(void *, GetDirectBufferAddress, JNIEnv *, env, jobject, obj)
DEFINE_WRAPPER2(jlong, GetDirectBufferCapacity, JNIEnv *, env, jobject, obj)
DEFINE_WRAPPER2(jobjectRefType, GetObjectRefType, JNIEnv *, env, jobject, obj)
DEFINE_WRAPPER2(jobject, GetModule, JNIEnv *, env, jclass, clazz)

struct JNIEnvWrapper wrappers = {
    NULL,
    NULL,
    NULL,
    NULL,
    WrapperGetVersion,
    WrapperDefineClass,
    WrapperFindClass,
    WrapperFromReflectedMethod,
    WrapperFromReflectedField,
    WrapperToReflectedMethod,
    WrapperGetSuperclass,
    WrapperIsAssignableFrom,
    WrapperToReflectedField,
    WrapperThrow,
    WrapperThrowNew,
    WrapperExceptionOccurred,
    WrapperExceptionDescribe,
    WrapperExceptionClear,
    WrapperFatalError,
    WrapperPushLocalFrame,
    WrapperPopLocalFrame,
    WrapperNewGlobalRef,
    WrapperDeleteGlobalRef,
    WrapperDeleteLocalRef,
    WrapperIsSameObject,
    WrapperNewLocalRef,
    WrapperEnsureLocalCapacity,
    WrapperAllocObject,
    WrapperNewObject,
    WrapperNewObjectV,
    WrapperNewObjectA,
    WrapperGetObjectClass,
    WrapperIsInstanceOf,
    WrapperGetMethodID,
    WrapperCallObjectMethod,
    WrapperCallObjectMethodV,
    WrapperCallObjectMethodA,
    WrapperCallBooleanMethod,
    WrapperCallBooleanMethodV,
    WrapperCallBooleanMethodA,
    WrapperCallByteMethod,
    WrapperCallByteMethodV,
    WrapperCallByteMethodA,
    WrapperCallCharMethod,
    WrapperCallCharMethodV,
    WrapperCallCharMethodA,
    WrapperCallShortMethod,
    WrapperCallShortMethodV,
    WrapperCallShortMethodA,
    WrapperCallIntMethod,
    WrapperCallIntMethodV,
    WrapperCallIntMethodA,
    WrapperCallLongMethod,
    WrapperCallLongMethodV,
    WrapperCallLongMethodA,
    WrapperCallFloatMethod,
    WrapperCallFloatMethodV,
    WrapperCallFloatMethodA,
    WrapperCallDoubleMethod,
    WrapperCallDoubleMethodV,
    WrapperCallDoubleMethodA,
    WrapperCallVoidMethod,
    WrapperCallVoidMethodV,
    WrapperCallVoidMethodA,
    WrapperCallNonvirtualObjectMethod,
    WrapperCallNonvirtualObjectMethodV,
    WrapperCallNonvirtualObjectMethodA,
    WrapperCallNonvirtualBooleanMethod,
    WrapperCallNonvirtualBooleanMethodV,
    WrapperCallNonvirtualBooleanMethodA,
    WrapperCallNonvirtualByteMethod,
    WrapperCallNonvirtualByteMethodV,
    WrapperCallNonvirtualByteMethodA,
    WrapperCallNonvirtualCharMethod,
    WrapperCallNonvirtualCharMethodV,
    WrapperCallNonvirtualCharMethodA,
    WrapperCallNonvirtualShortMethod,
    WrapperCallNonvirtualShortMethodV,
    WrapperCallNonvirtualShortMethodA,
    WrapperCallNonvirtualIntMethod,
    WrapperCallNonvirtualIntMethodV,
    WrapperCallNonvirtualIntMethodA,
    WrapperCallNonvirtualLongMethod,
    WrapperCallNonvirtualLongMethodV,
    WrapperCallNonvirtualLongMethodA,
    WrapperCallNonvirtualFloatMethod,
    WrapperCallNonvirtualFloatMethodV,
    WrapperCallNonvirtualFloatMethodA,
    WrapperCallNonvirtualDoubleMethod,
    WrapperCallNonvirtualDoubleMethodV,
    WrapperCallNonvirtualDoubleMethodA,
    WrapperCallNonvirtualVoidMethod,
    WrapperCallNonvirtualVoidMethodV,
    WrapperCallNonvirtualVoidMethodA,
    WrapperGetFieldID,
    WrapperGetObjectField,
    WrapperGetBooleanField,
    WrapperGetByteField,
    WrapperGetCharField,
    WrapperGetShortField,
    WrapperGetIntField,
    WrapperGetLongField,
    WrapperGetFloatField,
    WrapperGetDoubleField,
    WrapperSetObjectField,
    WrapperSetBooleanField,
    WrapperSetByteField,
    WrapperSetCharField,
    WrapperSetShortField,
    WrapperSetIntField,
    WrapperSetLongField,
    WrapperSetFloatField,
    WrapperSetDoubleField,
    WrapperGetStaticMethodID,
    WrapperCallStaticObjectMethod,
    WrapperCallStaticObjectMethodV,
    WrapperCallStaticObjectMethodA,
    WrapperCallStaticBooleanMethod,
    WrapperCallStaticBooleanMethodV,
    WrapperCallStaticBooleanMethodA,
    WrapperCallStaticByteMethod,
    WrapperCallStaticByteMethodV,
    WrapperCallStaticByteMethodA,
    WrapperCallStaticCharMethod,
    WrapperCallStaticCharMethodV,
    WrapperCallStaticCharMethodA,
    WrapperCallStaticShortMethod,
    WrapperCallStaticShortMethodV,
    WrapperCallStaticShortMethodA,
    WrapperCallStaticIntMethod,
    WrapperCallStaticIntMethodV,
    WrapperCallStaticIntMethodA,
    WrapperCallStaticLongMethod,
    WrapperCallStaticLongMethodV,
    WrapperCallStaticLongMethodA,
    WrapperCallStaticFloatMethod,
    WrapperCallStaticFloatMethodV,
    WrapperCallStaticFloatMethodA,
    WrapperCallStaticDoubleMethod,
    WrapperCallStaticDoubleMethodV,
    WrapperCallStaticDoubleMethodA,
    WrapperCallStaticVoidMethod,
    WrapperCallStaticVoidMethodV,
    WrapperCallStaticVoidMethodA,
    WrapperGetStaticFieldID,
    WrapperGetStaticObjectField,
    WrapperGetStaticBooleanField,
    WrapperGetStaticByteField,
    WrapperGetStaticCharField,
    WrapperGetStaticShortField,
    WrapperGetStaticIntField,
    WrapperGetStaticLongField,
    WrapperGetStaticFloatField,
    WrapperGetStaticDoubleField,
    WrapperSetStaticObjectField,
    WrapperSetStaticBooleanField,
    WrapperSetStaticByteField,
    WrapperSetStaticCharField,
    WrapperSetStaticShortField,
    WrapperSetStaticIntField,
    WrapperSetStaticLongField,
    WrapperSetStaticFloatField,
    WrapperSetStaticDoubleField,
    WrapperNewString,
    WrapperGetStringLength,
    WrapperGetStringChars,
    WrapperReleaseStringChars,
    WrapperNewStringUTF,
    WrapperGetStringUTFLength,
    WrapperGetStringUTFChars,
    WrapperReleaseStringUTFChars,
    WrapperGetArrayLength,
    WrapperNewObjectArray,
    WrapperGetObjectArrayElement,
    WrapperSetObjectArrayElement,
    WrapperNewBooleanArray,
    WrapperNewByteArray,
    WrapperNewCharArray,
    WrapperNewShortArray,
    WrapperNewIntArray,
    WrapperNewLongArray,
    WrapperNewFloatArray,
    WrapperNewDoubleArray,
    WrapperGetBooleanArrayElements,
    WrapperGetByteArrayElements,
    WrapperGetCharArrayElements,
    WrapperGetShortArrayElements,
    WrapperGetIntArrayElements,
    WrapperGetLongArrayElements,
    WrapperGetFloatArrayElements,
    WrapperGetDoubleArrayElements,
    WrapperReleaseBooleanArrayElements,
    WrapperReleaseByteArrayElements,
    WrapperReleaseCharArrayElements,
    WrapperReleaseShortArrayElements,
    WrapperReleaseIntArrayElements,
    WrapperReleaseLongArrayElements,
    WrapperReleaseFloatArrayElements,
    WrapperReleaseDoubleArrayElements,
    WrapperGetBooleanArrayRegion,
    WrapperGetByteArrayRegion,
    WrapperGetCharArrayRegion,
    WrapperGetShortArrayRegion,
    WrapperGetIntArrayRegion,
    WrapperGetLongArrayRegion,
    WrapperGetFloatArrayRegion,
    WrapperGetDoubleArrayRegion,
    WrapperSetBooleanArrayRegion,
    WrapperSetByteArrayRegion,
    WrapperSetCharArrayRegion,
    WrapperSetShortArrayRegion,
    WrapperSetIntArrayRegion,
    WrapperSetLongArrayRegion,
    WrapperSetFloatArrayRegion,
    WrapperSetDoubleArrayRegion,
    WrapperRegisterNatives,
    WrapperUnregisterNatives,
    WrapperMonitorEnter,
    WrapperMonitorExit,
    WrapperGetJavaVM,
    WrapperGetStringRegion,
    WrapperGetStringUTFRegion,
    WrapperGetPrimitiveArrayCritical,
    WrapperReleasePrimitiveArrayCritical,
    WrapperGetStringCritical,
    WrapperReleaseStringCritical,
    WrapperNewWeakGlobalRef,
    WrapperDeleteWeakGlobalRef,
    WrapperExceptionCheck,
    WrapperNewDirectByteBuffer,
    WrapperGetDirectBufferAddress,
    WrapperGetDirectBufferCapacity,
    WrapperGetObjectRefType,
    WrapperGetModule,
};

void init_jni_wrapper() {
    int pagesz;
    size_t jnienv_struct_size;
    size_t envwrapper_struct_size;
    struct JNIEnvWrapper *envWrapper;

    jnienv_struct_size = sizeof(struct JNINativeInterface_);
    envwrapper_struct_size = sizeof(struct JNIEnvWrapper);
    if (jnienv_struct_size != envwrapper_struct_size) {
        fprintf(stderr, "JNI version not supported\n");
        exit(1);
    }

    pagesz = getpagesize();

    globalWrapper = (JNIWrapper *)mmap(NULL, pagesz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (globalWrapper == MAP_FAILED) {
        perror("mmap(globalWrapper)");
        exit(1);
    }

    envWrapper = (struct JNIEnvWrapper *)mmap(NULL, pagesz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (envWrapper == MAP_FAILED) {
        perror("mmap(envWrapper)");
        exit(1);
    }
    memcpy(envWrapper, &wrappers, sizeof(wrappers));
    *globalWrapper = envWrapper;

    pkey_mprotect(envWrapper, pagesz, PROT_READ, LOADER_DOMAIN);
    pkey_mprotect(globalWrapper, pagesz, PROT_READ, LOADER_DOMAIN);

    to_c = new_hash_table(0x1000);
    to_java = new_hash_table(0x1000);
}

void set_domain_env(int domain, JNIEnv *env) {
    domainEnv[domain] = env;
}