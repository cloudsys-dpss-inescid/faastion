#include <stdio.h>

int inc(int a) { 
  fprintf(stderr, "musl a = %d\n", a);
  return ++a; 
}
