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

void addParam(CommandNode** cmd, char* parameter) {
    // printf("Adicionando parametro %s\n", parameter);
    CommandNode* firstCommand = *cmd;
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

void addCommand(CommandNode** ini, char* command) {
    // printf("Adicionando comando %s\n", command);
    CommandNode* new = createCommandNode(command);
    if (*ini == NULL) {
        *ini = new;
    }
    else {
        CommandNode* current = *ini;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new;
    }
}

void printOriginal(char *buf, CommandNode* ini) {
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

void printCommandList(CommandNode* ini) {
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
    if (ini != NULL) {
        freeParamList(ini->next);
        free(ini);
        ini = NULL;
    }
    return;
}

void freeCommandList(CommandNode* ini) {
    if (ini != NULL) {
        freeCommandList(ini->next);
        freeParamList(ini->paramList);
        free(ini);
        ini = NULL;
    }
}

void cleanupList(CommandNode *list) {
    if (list != NULL) {
        // printCommandList(list);
        freeCommandList(list);
    }
}