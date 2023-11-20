%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "http.h"
#include "lists.h"

extern CommandNode* mainList;
extern char webSpacePath[50];
extern int logfile;

%}

%union {
    char word[100];
}
%token <word> COMMAND ARG HOST_PORT
%token COLON NEWLINE COMMA
%type <word> request

%%

requests : requests request
         | request
         ;

request : command_line NEWLINE { sendRequest($$); }
        | command_line param_lines NEWLINE { sendRequest($$); }
        ;

command_line: COMMAND NEWLINE { splitCommand($1); } 

param_lines : param_lines param_line NEWLINE
            | param_line NEWLINE
            ;

param_line : param_line COMMA ARG { addParam(&mainList, $3); }
           | ARG COLON ARG { addCommand(&mainList, $1); addParam(&mainList, $3); }
           | ARG COLON HOST_PORT { addCommand(&mainList, $1); addParam(&mainList, $3); }
           ;

%%

void sendRequest(char *request) {
    processRequisition(mainList->command, webSpacePath, mainList->paramList->parameter);
    cleanupList(mainList);
    mainList = NULL;
    write(logfile, "--------------------------------------------------\n\n", 52);
}

void splitCommand(char *text) {
    char *tok = strtok(text, " ");
    addCommand(&mainList, tok);
    tok = strtok(NULL, " ");
    addParam(&mainList, tok);
    tok = strtok(NULL, " ");
    addParam(&mainList, tok);
}
