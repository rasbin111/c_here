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

  printf("\n");

  int *ptr2 = calloc(5, sizeof(int)); // memory items initialized with zero

  if (ptr2 == NULL) {
    printf("Memory allocation fialed\n");
    free(ptr);
    return 1;
  }

  for (int i = 0; i < 5; i++) {
    printf("%d ", ptr[i]);
  }
  printf("\n");

  printf("size of ptr2 before realloc: %zu\n", sizeof(ptr2));

  int *temp = realloc(ptr2, 10 * sizeof(int));

  if (temp == NULL) {
    printf("Memory reallocation failed\n");
    free(ptr2);
    return 1;
  }

  ptr2 = temp;

  printf("size of ptr2 after realloc: %zu\n", sizeof(ptr2));

  free(ptr);
  free(ptr2);

  return 0;
}
