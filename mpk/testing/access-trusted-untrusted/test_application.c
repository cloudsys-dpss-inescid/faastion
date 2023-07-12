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

int inc(int a) { return a++; }

void wrapper() {
  // TODO - extract arguments from shared storage;
  // TODO - call inc
  // TODO - insert arguments back in storage;
}

int main(int argc, char **argv) {
  if(erim_init(8192, ERIM_FLAG_ISOLATE_TRUSTED | ERIM_FLAG_SWAP_STACK)) {
    exit(EXIT_FAILURE);
  }
  __wrpkru(ERIM_TRUSTED_PKRU);
  // TODO - copy in arguments

  ERIM_SWITCH_TO_ISOLATED_STACK;
  __wrpkru(ERIM_UNTRUSTED_PKRU);
  wrapper();
  __wrpkru(ERIM_TRUSTED_PKRU);
  ERIM_SWITCH_TO_REGULAR_STACK;

  // TODO - copy out arguments

  return 0;
}
