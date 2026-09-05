#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE* fptr;
    char data[50];
    fptr = fopen("players.txt", "r");
    if (fptr == NULL) {
        printf("File failed to open.\n");
        return 1;
    }
    printf("File is now opened.\n");
    while (fgets(data, 50, fptr)) {
        printf("%s", data);
    }
    return 0;
}
