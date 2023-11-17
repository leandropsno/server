%{
#include "http.tab.h"
#include <string.h>
#include <stdio.h>
extern FILE *logfile;
%}

COMMAND     (GET|HEAD|OPTIONS|TRACE)[ ][^ :,\t\r\n]+[ ][^ :,\t\r\n]+
ARG         [^:,\t\r\n]+
HOST_PORT   [^ :,\t\r\n]+:[0-9]{1,5}
COLON       [:][ ]
COMMA       [,]
NEWLINE     (\r\n)

%%

{COMMAND}		{ fprintf(logfile, "---------- NOVO PAR REQUISIÇÃO/RESPOSTA ----------\n\n"); strcpy(yylval.word, yytext); fprintf(logfile, "%s", yytext); return COMMAND; }
{ARG}           { strcpy(yylval.word, yytext); fprintf(logfile, "%s", yytext); return ARG; }
{COLON}         { fprintf(logfile, "%s", yytext); return COLON; }
{COMMA}         { fprintf(logfile, "%s", yytext); return COMMA; }
{HOST_PORT}     { strcpy(yylval.word, yytext); fprintf(logfile, "%s", yytext); return HOST_PORT; }
{NEWLINE}       { fprintf(logfile, "%s", yytext); return NEWLINE; }
.               ;

%%

