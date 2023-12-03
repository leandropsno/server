#include <stdio.h>
#include <stdlib.h>
#include "lists.h"

void limpar(listptr mainList) {
    CommandNode *list = *mainList;
    free(list->next);
    list->next = NULL;
    free(list);
    list = NULL;
    free(mainList);
    mainList = NULL;
}

int main() {

    listptr mainList = (listptr)malloc(sizeof(CommandNode *));
    addCommand(mainList, "Arrozcomfeijao");
    addCommand(mainList, "batatdocefrango");
    limpar(mainList);
    return 0;
}