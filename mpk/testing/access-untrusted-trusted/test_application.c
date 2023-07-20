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
  if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED)) {
    exit(EXIT_FAILURE);
  }
  erim_switch_to_untrusted;

  // allocate secret
  unsigned long * var = (unsigned long *) erim_malloc(sizeof(unsigned long));
  if(var == NULL) {
    printf("allocation of secret failed\n");
    exit(EXIT_FAILURE);
  }
  
  erim_switch_to_trusted;
  return var;
}

int main(int argc, char **argv) {
  unsigned long * var = create_secret_var();

  printf("var located at %p\n", var);

  // try to read, shouldn't work (not trusted)
  fprintf(stderr, "should segfault:\n");
  fprintf(stderr, "var: %lx\n", read_var(var));

  return SWS_SUCCESS;
}
