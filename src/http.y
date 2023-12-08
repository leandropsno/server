%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "lists.h"
#include "http.h"

void splitCommandLine();
void splitParamLine();

extern int logfile;
%}

%union {
    char word[100];
}
%token <word> COMMAND ARG HOST_PORT NEWLINE
%type <word> request command_line param_lines param_line
%parse-param { listptr mainList } { int *result } { int socket }
%%

request : command_line NEWLINE { *result = processRequest(mainList, socket); }
        | command_line param_lines NEWLINE { *result = processRequest(mainList, socket); }
        ;


command_line: COMMAND NEWLINE {  splitCommandLine(mainList, $1); }
            ;

param_lines : param_lines param_line 
            | param_line
            ;

param_line : ARG NEWLINE { splitParamLine(mainList, $1); }
           | HOST_PORT NEWLINE { splitParamLine(mainList, $1); }
           ;

%%

void splitCommandLine(listptr mainList, char *text) {
    char *saveptr;
    char *tok = strtok_r(text, " ", &saveptr);
    addCommand(mainList, tok);
    tok = strtok_r(NULL, " ", &saveptr);
    while (tok != NULL) {
        addParam(mainList, tok);
        tok = strtok_r(NULL, " ", &saveptr);
    }
}

void splitParamLine(listptr mainList, char *text) {
    char *saveptr;
    char *tok = strtok_r(text, ": ", &saveptr);
    addCommand(mainList, tok);
    tok = strtok_r(NULL, ",", &saveptr);
    while (tok != NULL) {
        addParam(mainList, tok);
        tok = strtok_r(NULL, ",", &saveptr);
    }
}