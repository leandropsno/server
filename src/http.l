%{
#include <string.h>
#include <stdio.h>
#include "lists.h"
#include "http.tab.h"
%}

COMMAND     (GET|HEAD|OPTIONS|TRACE)[ ][^ :,\t\r\n]+[ ][^ :,\t\r\n]+
HOST_PORT   (Host)[:][ ][^ :,\t\r\n]+:[0-9]{1,5}
ARG         [^,\t\r\n]+[:][ ][^\t\r\n]+
NEWLINE     (\r\n)

%%

{COMMAND}		{ strcpy(yylval.word, yytext); return COMMAND;  }
{HOST_PORT}     { strcpy(yylval.word, yytext); return HOST_PORT; }
{ARG}           { strcpy(yylval.word, yytext); return ARG; }
{NEWLINE}       { strcpy(yylval.word, yytext); return NEWLINE; }
.               { printf("Sequência não reconhecida: ");
                    for (int i = 0; i < yyleng; i++) {
                    printf("0x%x ", (int)yytext[i]); } 
                    printf("\n"); }

%%

