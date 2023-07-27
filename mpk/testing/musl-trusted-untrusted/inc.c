#include <stdio.h>
#include<unistd.h>

int inc(int a) {
  fprintf(stderr, "a = %d\n", a);
  return ++a; 
}
