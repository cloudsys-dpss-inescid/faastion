#include "../../../build/generated/sources/headers/java/main/AesEncryption.h"
#include <string.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

int aes_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
  unsigned char *iv, unsigned char *ciphertext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    if(!(ctx = EVP_CIPHER_CTX_new())) return -1;

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return -1;

    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        return -1;
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) return -1;
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

int aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
  unsigned char *iv, unsigned char *plaintext)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    if(!(ctx = EVP_CIPHER_CTX_new())) return -1;

    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        return -1;

    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        return -1;
    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) return -1;
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

JNIEXPORT jint JNICALL Java_AesEncryption_aes
  (JNIEnv * env, jobject obj){
  
  	
  
  unsigned char plaintext[] = "This is a secret message!";
  unsigned char key[] = "01234567890123456789012345678901";
  unsigned char iv[] = "0123456789012345";
  unsigned char ciphertext[128];
  unsigned char decryptedtext[128];
  int decryptedtext_len, ciphertext_len;

  ciphertext_len = aes_encrypt (plaintext, strlen ((char *)plaintext), key,
                                iv, ciphertext);

  printf("Ciphertext is:\n");
  BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

  decryptedtext_len = aes_decrypt(ciphertext, ciphertext_len, key,
                                  iv, decryptedtext);

  decryptedtext[decryptedtext_len] = '\0';

  printf("Decrypted text is:\n");
  printf("%s\n", decryptedtext);
  return 0;
 
  }
