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

unsigned long read_var(unsigned long * var) {
  return *var;
}

unsigned long * create_secret_var() {
  // init isolation and sh mem
  if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED | ERIM_FLAG_SWAP_STACK)) {
    exit(EXIT_FAILURE);
  }
  erim_switch_to_trusted;

  fprintf(stderr, "domain: %x\n", __rdpkru());

  fprintf(stderr, "domain: %d\n", ERIM_EXEC_DOMAIN(__rdpkru()));

  // allocate secret
  unsigned long * var = (unsigned long *) erim_malloc(sizeof(unsigned long));
  if(var == NULL) {
    printf("allocation of secret failed\n");
    exit(EXIT_FAILURE);
  }
  *var = 123;
  
  return var;
}

void test() {
  unsigned long ola = 10;
}

int main(int argc, char **argv) {
  unsigned long * var = create_secret_var();

  erim_switch_to_monitor; 

  printf("var located at %p\n", var);
  //unsigned long * trustedVar = (unsigned long *) erim_mallocIsolated(sizeof(unsigned long));
  //*trustedVar = *var;
  //fprintf(stderr, "trustedVar: %ld\n", read_var(trustedVar));
  
  //erim_switch_to_trusted;
  //__wrpkru(ERIM_UNTRUSTED_PKRU);
  //ERIM_SWITCH_TO_UNTRUSTED_STACK; 

  //unsigned long test = var2;
  // *trustedVar = 10;
  test();
  erim_switch_to_trusted;

  // try to read, shouldn't work (not trusted)
  fprintf(stderr, "Success\n");

  return SWS_SUCCESS;
}
