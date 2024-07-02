#include "com_jni_FileHashing.h"
#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>


JNIEXPORT void JNICALL Java_com_jni_FileHashing_hash(JNIEnv *env, jobject object) {
    const char *filename = "~/snap.png";
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    MD5_CTX md5Context;
    MD5_Init(&md5Context);

    unsigned char data[1024];
    size_t bytesRead;
    while ((bytesRead = fread(data, 1, sizeof(data), file)) > 0) {
        MD5_Update(&md5Context, data, bytesRead);
    }

    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, &md5Context);

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        fprintf(stderr, "%02x", hash[i]);
    }
    fprintf(stderr, "\n");

    fclose(file);
}
