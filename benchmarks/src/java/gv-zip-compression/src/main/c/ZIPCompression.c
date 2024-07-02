#include "com_jni_ZIPCompression.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>

void printHex(const unsigned char *data, int len) {
    for (int i = 0; i < len; ++i) {
        fprintf(stderr, "%02x", data[i]);
    }
    fprintf(stderr, "\n");
}

JNIEXPORT void JNICALL Java_com_jni_ZIPCompression_compress(JNIEnv *env, jobject object) {
    const char *filename = "~/snap.png";
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *inputData = (unsigned char *)malloc(fileSize);
    if (!inputData) {
        perror("Memory allocation failed");
        return 1;
    }

    if (fread(inputData, 1, fileSize, file) != fileSize) {
        perror("Reading input file failed");
        return 1;
    }
    fclose(file);

    uLong compressedDataSize = compressBound(fileSize);
    unsigned char *compressedData = (unsigned char *)malloc(compressedDataSize);
    if (!compressedData) {
        perror("Memory allocation failed");
        return 1;
    }

    if (compress(compressedData, &compressedDataSize, inputData, fileSize) != Z_OK) {
        handleErrors();
    }

    printHex(compressedData, compressedDataSize);

    free(inputData);
    free(compressedData);

    return 0;
}
