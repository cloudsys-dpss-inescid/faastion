/*
 * test_application.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>

// Erim includes
#include <common.h>
#include <erim.h>

int inc(int arg) { return arg++; }

int main(int argc, char **argv) {

  if(erim_init(8192, ERIM_FLAG_ISOLATE_TRUSTED | ERIM_FLAG_SWAP_STACK)) {
    exit(EXIT_FAILURE);
  }
#ifdef ERIM_SWAP_STACKS
  printf("ERIM_SWAP_STACKS is defined!\n");
#endif

  ERIM_SWITCH_TO_UNTRUSTED_STACK;
  __wrpkru(ERIM_UNTRUSTED_PKRU);

  int ret = inc(3);
  
  __wrpkru(ERIM_TRUSTED_PKRU);
  ERIM_SWITCH_TO_TRUSTED_STACK;

  return 0;
}
