#include <stdio.h>

void swap(int *a, int *b) {
  int c = *a;
  *a = *b;
  *b = c;
}

int main() {
  int a = 10;
  int *ptr_int = &a;

  printf("Ptr to a %p\n", ptr_int);

  int s1 = 99;
  int s2 = 77;
  swap(&s1, &s2);

  printf("After swap: s1 = %d, s2 = %d\n", s1, s2);

  return 0;
}
