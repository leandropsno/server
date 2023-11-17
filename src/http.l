%{
#include "http.tab.h"
#include <string.h>
#include <stdio.h>
extern FILE *registro;
%}

COMANDO     (GET|HEAD|OPTIONS|TRACE)[ ][^ :,\t\r\n]+[ ][^ :,\t\r\n]+
ARG         [^:,\t\r\n]+
HOST_PORT   [^ :,\t\r\n]+:[0-9]{1,5}
DOIS_PONTOS [:][ ]
VIRGULA     [,]
NEWLINE     (\r\n)

%%

{COMANDO}		{ fprintf(registro, "---------- NOVO PAR REQUISIÇÃO/RESPOSTA ----------\n\n"); strcpy(yylval.word, yytext); fprintf(registro, "%s", yytext); return COMANDO; }
{ARG}           { strcpy(yylval.word, yytext); fprintf(registro, "%s", yytext); return ARG; }
{DOIS_PONTOS}   { fprintf(registro, "%s", yytext); return DOIS_PONTOS; }
{VIRGULA}       { fprintf(registro, "%s", yytext); return VIRGULA; }
{HOST_PORT}     { strcpy(yylval.word, yytext); fprintf(registro, "%s", yytext); return HOST_PORT; }
{NEWLINE}       { fprintf(registro, "%s", yytext); return NEWLINE; }
.               ;

%%

