#include <stdio.h>

void printFuncMacro() { printf("Func: %s\n", __func__); }

int main() {
  printf("Date : %s\n", __DATE__);
  printf("Time: %s\n", __TIME__);
  printf("File: %s\n", __FILE__);
  printf("Line: %d\n", __LINE__);
  printFuncMacro();
  return 0;
}
