#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE *fp;
  struct emp {
    char name[40];
    int age;
    float bs;
  };
  struct emp e;
  char ch = 'Y';

  fp = fopen("EMPLOYEE.DAT", "w");

  while (ch == 'Y') {
    printf("Enter name, age, salary:");
    scanf("%s %d %f", e.name, &e.age, &e.bs);
    getchar();
    fprintf(fp, "%s %d %f\n", e.name, e.age, e.bs);
    printf("Another record:");
    ch = getchar();
  }

  fclose(fp);

  fp = fopen("EMPLOYEE.DAT", "r");

  while (fscanf(fp, "%s %d %f", e.name, &e.age, &e.bs) != EOF) {
    printf("%s %d %f\n", e.name, e.age, e.bs);
  }
  fclose(fp);
  return 0;
};
