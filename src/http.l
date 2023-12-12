%{
#include <string.h>
#include <stdio.h>
#include "lists.h"
#include "http.tab.h"

extern int logfile;
%}

COMMAND     (GET|HEAD|OPTIONS|TRACE|POST)[ ][^ :,\t\r\n]+[ ][^ :,\t\r\n]+
HOST_PORT   (Host)[:][ ][^ :,\t\r\n]+:[0-9]{1,5}
ARG         [^,\t\r\n]+[:][ ][^\r\n]+
NEWLINE     [\r][\n]

%%

{COMMAND}		{ write(logfile, yytext, yyleng); strcpy(yylval.word, yytext); return COMMAND;  }
{HOST_PORT}     { write(logfile, yytext, yyleng); strcpy(yylval.word, yytext); return HOST_PORT; }
{ARG}           { write(logfile, yytext, yyleng); strcpy(yylval.word, yytext); return ARG; }
{NEWLINE}       { write(logfile, yytext, yyleng); strcpy(yylval.word, yytext); return NEWLINE; }

%%

