#include "com_jni_AESEncryption.h"
#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>

const unsigned char key[16] = "0123456789abcdef";

void printHex(const unsigned char *data, int len) {
    for (int i = 0; i < len; ++i) {
        fprintf(stderr, "%02x", data[i]);
    }
    fprintf(stderr, "\n");
}

JNIEXPORT void JNICALL Java_com_jni_AESEncryption_cipher(JNIEnv *env, jobject object) {
    const char *filename = "~/snap.png";
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    AES_KEY encryptKey;
    if (AES_set_encrypt_key(key, 128, &encryptKey) < 0) {
        fprintf(stderr, "An error occurred\n");
        exit(1);    
    }

    unsigned char inbuf[AES_BLOCK_SIZE];
    unsigned char outbuf[AES_BLOCK_SIZE];
    int numBytesRead;
    while ((numBytesRead = fread(inbuf, 1, AES_BLOCK_SIZE, file)) > 0) {
        if (numBytesRead < AES_BLOCK_SIZE) {
            for (int i = numBytesRead; i < AES_BLOCK_SIZE; ++i) {
                inbuf[i] = 0;
            }
        }
        AES_encrypt(inbuf, outbuf, &encryptKey);
        printHex(outbuf, AES_BLOCK_SIZE);
    }
    
    fclose(file);
}
