#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lists.h"

ParamNode* createParamNode(char* parameter) {
    ParamNode* new = (ParamNode*)malloc(sizeof(ParamNode));
    if (new != NULL) {
        strcpy(new->parameter, parameter);
        new->next = NULL;
    }
    else {
        exit(1);
    }
    return new;
}

CommandNode* createCommandNode(char* command) {
    CommandNode* new = (CommandNode*)malloc(sizeof(CommandNode));
    if (new != NULL) {
        strcpy(new->command, command);
        new->paramList = NULL;
        new->next = NULL;
    }
    else {
        exit(1);
    }
    return new;
}

void addParam(listptr list, char* parameter) {
    CommandNode* firstCommand = *list;
    while (firstCommand->next != NULL) {
        firstCommand = firstCommand->next;
    }
    ParamNode* new = createParamNode(parameter);
    if (firstCommand->paramList == NULL) {
        firstCommand->paramList = new;
    }
    else {
        ParamNode* current = firstCommand->paramList;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new;
    }
}

void addCommand(listptr list, char* command) {
    CommandNode* new = createCommandNode(command);
    if (*list == NULL) {
        *list = new;
    }
    else {
        CommandNode* current = *list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new;
    }
}

void printOriginal(char *buf, listptr list) {
    CommandNode* ini = *list;
    int w = sprintf(buf, "%s %s %s\n", ini->command, ini->paramList->parameter, ini->paramList->next->parameter);
    CommandNode* currentCommand = ini->next;
    while (currentCommand != NULL) {
        w += sprintf(&buf[w], "%s:", currentCommand->command);
        ParamNode* currentParam = currentCommand->paramList;
        while (currentParam != NULL) {
            w += sprintf(&buf[w], " %s", currentParam->parameter);
            currentParam = currentParam->next;
        }
        w += sprintf(&buf[w], "\n");
        currentCommand = currentCommand->next;
    }
}

void printCommandList(listptr list) {
    CommandNode* ini = *list;
    printf("-------------- NOVO PAR REQUISICAO/RESPOSTA -------------\n");
    CommandNode* currentCommand = ini;
    while (currentCommand != NULL) {
        printf("Chave: %s\n", currentCommand->command);
        ParamNode* currentParam = currentCommand->paramList;
        while (currentParam != NULL) {
            printf("   Valor: %s\n", currentParam->parameter);
            currentParam = currentParam->next;
        }
        currentCommand = currentCommand->next;
    }
    printf("---------------------------------------------------------\n\n");
}

void freeParamList(ParamNode* ini) {
    ParamNode *current = ini;
    ParamNode *next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}

void freeCommandList(CommandNode* ini) {
    CommandNode *current = ini;
    CommandNode *next;
    while (current != NULL) {
        next = current->next;
        freeParamList(current->paramList);
        free(current);
        current = next;
    }
    ini = NULL;
}

void cleanupList(listptr list) {
    freeCommandList(*list);
    free(list);
    list = NULL;
}
