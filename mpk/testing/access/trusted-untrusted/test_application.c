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

static __thread char* regular = NULL;

int inc(int a) { 
  return ++a; 
}

int wrapper(int a) {
  int ret = 123;
  __wrpkru(ERIM_DOMAIN(1));
  ret = inc(a);
  __wrpkru(ERIM_DOMAIN(0));
  return ret;
}

int main(int argc, char **argv) {
  int a = 321;

  // trusted (regular) domain -> 0 (can access both domains 0 and 1, pkru = 0x55555550)
  // untrusted (isolated) domain -> 1 (con only access domain 1, pkry = 0x55555553)
  if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED | ERIM_FLAG_SWAP_STACK, 2)) {
    exit(EXIT_FAILURE);
  }

  ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(1), regular);
  a = wrapper(a);
  ERIM_SWITCH_BACK(regular);
  fprintf(stderr, "a = %d\n", a);
  return 0;
}
