%{
#include <string.h>
#include <stdio.h>
#include "lists.h"
#include "http.tab.h"
%}

COMMAND     (GET|HEAD|OPTIONS|TRACE)[ ][^ :,\t\r\n]+[ ][^ :,\t\r\n]+
ARG         [^:,\t\r\n]+
HOST_PORT   [^ :,\t\r\n]+:[0-9]{1,5}
COLON       [:][ ]
COMMA       [,]
NEWLINE     (\r\n)

%%

{COMMAND}		{ strcpy(yylval.word, yytext); return COMMAND;  }
{ARG}           { strcpy(yylval.word, yytext); return ARG; }
{COLON}         { return COLON; }
{COMMA}         { return COMMA; }
{HOST_PORT}     { strcpy(yylval.word, yytext);  return HOST_PORT; }
{NEWLINE}       { return NEWLINE; }
.               { printf("Sequência não reconhecida: ");
                    for (int i = 0; i < yyleng; i++) {
                    printf("0x%x ", (int)yytext[i]); } 
                    printf("\n"); }

%%

