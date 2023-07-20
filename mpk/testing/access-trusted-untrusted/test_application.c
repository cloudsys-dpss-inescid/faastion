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

int inc(int a) { 
  __wrpkru(ERIM_TRUSTED_PKRU);
  fprintf(stderr, "a = %d\n", a);
  __wrpkru(ERIM_UNTRUSTED_PKRU);

  return ++a; 
}

int wrapper(int a) {
  int ret = 123;
  __wrpkru(ERIM_UNTRUSTED_PKRU);
  ret = inc(a);
  __wrpkru(ERIM_TRUSTED_PKRU);
  return ret;
}

int main(int argc, char **argv) {
  int a = 321;

  // trusted (regular) domain -> 0 (can access both domains 0 and 1, pkru = 0x55555550)
  // untrusted (isolated) domain -> 1 (con only access domain 1, pkry = 0x55555553)
  if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED | ERIM_FLAG_SWAP_STACK)) {
    exit(EXIT_FAILURE);
  }

  ERIM_SWITCH_TO_ISOLATED_STACK;
  a = wrapper(a);
  ERIM_SWITCH_TO_REGULAR_STACK;
  fprintf(stderr, "a = %d\n", a);
  return 0;
}
