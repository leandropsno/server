%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http.h"
#include "lists.h"

extern CommandNode* mainList;
extern char webSpacePath[50];
extern FILE *registro;

%}

%union {
    char word[100];
}
%token <word> COMANDO ARG HOST_PORT
%token DOIS_PONTOS NEWLINE VIRGULA
%type <word> requisicao

%%

requisicoes : requisicoes requisicao
            | requisicao
            ;

requisicao : linha_comando NEWLINE { enviarRequisicao($$); }
           | linha_comando linhas_parametro NEWLINE { enviarRequisicao($$); }
           ;

linha_comando: COMANDO NEWLINE { destrincharComando($1); } 

linhas_parametro : linhas_parametro linha_parametro NEWLINE
                 | linha_parametro NEWLINE
                 ;

linha_parametro: linha_parametro VIRGULA ARG { addParam(&mainList, $3); }
               | ARG DOIS_PONTOS ARG { addCommand(&mainList, $1); addParam(&mainList, $3); }
               | ARG DOIS_PONTOS HOST_PORT { addCommand(&mainList, $1); addParam(&mainList, $3); }
               ;

%%

void enviarRequisicao(char *requisicao) {
    int code = processRequisition(webSpacePath, mainList->paramList->parameter);
    cleanupList(mainList);
    mainList = NULL;
    fprintf(registro, "--------------------------------------------------\n\n");
}

void destrincharComando(char *text) {
    char *tok = strtok(text, " ");
    addCommand(&mainList, tok);
    tok = strtok(NULL, " ");
    addParam(&mainList, tok);
    tok = strtok(NULL, " ");
    addParam(&mainList, tok);
}
