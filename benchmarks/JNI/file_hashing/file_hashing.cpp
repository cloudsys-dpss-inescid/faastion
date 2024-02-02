#include <stdio.h>
#include "file_hashing.h"
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <openssl/evp.h>
#include <chrono>

JNIEXPORT jstring JNICALL Java_file_1hashing_hashing
  (JNIEnv *env, jobject thisObj, jstring data){
        auto start = std::chrono::high_resolution_clock::now();
	const char* line = env->GetStringUTFChars(data,NULL);
	EVP_MD_CTX* ctx = EVP_MD_CTX_new();
	const EVP_MD* md5 = EVP_md5();
    	unsigned char digest[EVP_MAX_MD_SIZE];
	unsigned int digest_len;

    	EVP_DigestInit_ex(ctx, md5, nullptr);
	EVP_DigestUpdate(ctx, line, strlen(line));
	EVP_DigestFinal_ex(ctx, digest, &digest_len);
	std::stringstream ss;
	for (unsigned int i = 0; i < digest_len; ++i) {
        	ss << std::hex << static_cast<int>(digest[i]);
	}
	EVP_MD_CTX_free(ctx);
	std::string str1 = ss.str();
        auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	std::cout << static_cast<double> (duration.count()) / 1000000000.0 << std::endl;
	return env->NewStringUTF(str1.c_str());;

}

