%{
#include "http.tab.h"
#include <string.h>
#include <stdio.h>
extern int logfile;
%}

COMMAND     (GET|HEAD|OPTIONS|TRACE)[ ][^ :,\t\r\n]+[ ][^ :,\t\r\n]+
ARG         [^:,\t\r\n]+
HOST_PORT   [^ :,\t\r\n]+:[0-9]{1,5}
COLON       [:][ ]
COMMA       [,]
NEWLINE     (\r\n)

%%

{COMMAND}		{ write(logfile, yytext, yyleng); strcpy(yylval.word, yytext); return COMMAND;  }
{ARG}           { write(logfile, yytext, yyleng); strcpy(yylval.word, yytext); return ARG; }
{COLON}         { write(logfile, yytext, yyleng); return COLON; }
{COMMA}         { write(logfile, yytext, yyleng); return COMMA; }
{HOST_PORT}     { write(logfile, yytext, yyleng); strcpy(yylval.word, yytext);  return HOST_PORT; }
{NEWLINE}       { write(logfile, yytext, yyleng); return NEWLINE; }
.               { printf("Sequência não reconhecida: \"%s\"\n", yytext); }

%%

