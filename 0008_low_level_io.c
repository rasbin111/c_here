#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    char buffer[512], source[128], target[128];
    char catchNewline;
    int inhandle, outhandle, bytes;

    printf("\n Enter source file name");
    scanf("%s", source);
    getchar();

    inhandle = open(source, O_RDONLY);

    if (inhandle == -1) {
        printf("Can't open file\n");
        exit(-1);
    }

    printf("\nEnter target file name");
    scanf("%s", target);

    outhandle = open(target, O_CREAT | O_WRONLY, 0644);

    if (outhandle == -1) {
        printf("Can't create file\n");
        exit(-1);
    }

    while (1) {
        bytes = read(inhandle, buffer, 512);

        if (bytes > 0) {
            write(outhandle, buffer, bytes);
        } else {
            break;
        }
    }
    close(inhandle);
    close(outhandle);

    return 0;
}
