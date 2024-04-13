#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <jni.h>
#include <openssl/evp.h>
#include <time.h>
#include "FileHashing.h"

JNIEXPORT jstring JNICALL Java_FileHashing_hash(JNIEnv *env, jobject thisObj, jstring data) {
    const char *line = (*env)->GetStringUTFChars(env, data, NULL);
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    const EVP_MD *md5 = EVP_md5();
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    EVP_DigestInit_ex(ctx, md5, NULL);
    EVP_DigestUpdate(ctx, line, strlen(line));
    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);
    
    char hex_digest[digest_len * 2 + 1];
    for (unsigned int i = 0; i < digest_len; ++i) {
        sprintf(hex_digest + i * 2, "%02x", digest[i]);
    }
    hex_digest[digest_len * 2] = '\0';

    (*env)->ReleaseStringUTFChars(env, data, line);

    printf("Hashed Message: %s\n", hex_digest);

    return (*env)->NewStringUTF(env, hex_digest);
}


