#include <stdio.h>

int main() {
  FILE *f;

  char buffer[20];

  f = fopen("hello.py", "w");

  if (f == NULL) {
    printf("Error: could not open file. \n");
    return 1;
  }

  int i = 0;
  while (i < 10) {
    sprintf(buffer, "%d\n", i);
    fputs(buffer, f);
    i++;
  }
  fclose(f);
  return 0;
}
