#include <stdio.h>
#include <stdlib.h>

int main() {
  int *ptr = malloc(5 * sizeof(int)); // no memory initialized

  if (ptr == NULL) {
    printf("Memory allocation failed\n");
    return 1;
  }
  for (int i = 0; i < 5; i++) {
    ptr[i] = i + 1;
  }

  for (int i = 0; i < 5; i++) {
    printf("%d ", *ptr + i);
  }
  free(ptr);
  printf("\n");

  int *ptr2 = calloc(5, sizeof(int)); // memory items initialized with zero

  for (int i = 0; i < 5; i++) {
    printf("%d ", ptr[i]);
  }
  free(ptr);
  printf("\n");
  return 0;
}
