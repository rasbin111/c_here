#include <stdio.h>
#include <string.h>

struct EMPLOYEE {
    char name[20];
    int age;
    double salary;
};

int main()
{
    FILE* f;
    char empname[20];
    f = fopen("./EMPLOYEE.DAT", "rb");

    struct EMPLOYEE e;

    int rec_size = sizeof(e);

    printf("Enter name of employee to modify: ");
    scanf("%s", empname);
    rewind(f);

    while (fread(&e, rec_size, 1, f) == 1) {
        printf("name: %s\n", e.name);
        if (strcmp(e.name, empname) == 0) {
            printf("\nEnter new name, age &bs ");
            scanf("%s %d %lf", e.name, &e.age, &e.salary);
            fseek(f, -rec_size, SEEK_CUR);
            fwrite(&e, rec_size, 1, f);
            break;
        }
    }

    fclose(f);
    return 0;
}
