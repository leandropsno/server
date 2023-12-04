#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void *smth(void *pointer) {
    int a = (intptr_t)pointer;
    int b = 0;
    b++;
}

void getType(char *type, char *filename) {
    char *tok = strtok(filename, ".");
    while (tok != NULL) {
        strcpy(type, tok);
        tok = strtok(NULL, ".");
    }
}

int main() {

    char file[22] = "aaaa.fefefefefefe.html";
    char type[4];
    getType(type, file);
    printf("%s type: %s\n", file, type);

    return 0;
}