#include <preload.h>

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

  ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(1), regular);
  a = wrapper(a);
  ERIM_SWITCH_BACK(regular);
  fprintf(stderr, "a = %d\n", a);
  return 0;
}
