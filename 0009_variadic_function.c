#include <stdarg.h>
#include <stdio.h>

int add(int count, ...)
{
    int total = 0;
    va_list va;
    va_start(va, count); // start with arguements after "count"

    for (int i = 0; i < count; i++) {
        int n = va_arg(va, int);
        total += n;
    }
    va_end(va);
    return total;
}

int main()
{
    int sum1 = add(4, 1, 2, 3, 4);
    int sum2 = add(2, 22, 33);

    printf("Sum 1: %d\n", sum1);
    printf("Sum 2: %d\n", sum2);
}
