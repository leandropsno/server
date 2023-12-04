%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "lists.h"
#include "http.h"

extern int logfile;
%}

%union {
    char word[100];
}
%token <word> COMMAND ARG HOST_PORT
%token COLON NEWLINE COMMA
%type <word> request
%parse-param { listptr mainList } { int *result } { int socket }
%%

requests : requests request
         | request
         ;

request : command_line NEWLINE { *result = processRequest(mainList, socket); }
        | command_line param_lines NEWLINE { *result = processRequest(mainList, socket); }
        ;

command_line: COMMAND NEWLINE { splitCommand(mainList, $1); } 

param_lines : param_lines param_line NEWLINE
            | param_line NEWLINE
            ;

param_line : param_line COMMA ARG { addParam(mainList, $3); }
           | ARG COLON ARG { addCommand(mainList, $1); addParam(mainList, $3); }
           | ARG COLON HOST_PORT { addCommand(mainList, $1); addParam(mainList, $3); }
           ;

%%

void splitCommand(listptr mainList, char *text) {
    char *tok = strtok(text, " ");
    addCommand(mainList, tok);
    tok = strtok(NULL, " ");
    addParam(mainList, tok);
    tok = strtok(NULL, " ");
    addParam(mainList, tok);
}
