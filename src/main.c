#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void *smth(void *pointer) {
    int a = (intptr_t)pointer;
    int b = 0;
    b++;
}

int main() {

    int a = 10;
    smth((void *)(intptr_t)a);
    return 0;
}