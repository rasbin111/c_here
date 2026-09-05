#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE* fptr;
    fptr = fopen("players.txt", "w");

    char data[50] = "Bruno Fernandes is a good player.\n";

    if (fptr == NULL) {
        printf("The file is not opened.\n");
    } else {
        printf("The file is created successfully.\n");
        fputs(data, fptr);
        fclose(fptr);
        printf("Data is written successfully to the file.\n");
    }

    return 0;
}
