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

{COMMAND}		{
                    write(logfile, "---------- NOVO PAR REQUISIÇÃO/RESPOSTA ----------\n\n", 52);
                    strcpy(yylval.word, yytext);
                    write(logfile, yytext, yyleng); 
                    return COMMAND;
                }
{ARG}           { strcpy(yylval.word, yytext); write(logfile, yytext, yyleng); return ARG; }
{COLON}         { write(logfile, yytext, yyleng); return COLON; }
{COMMA}         { write(logfile, yytext, yyleng); return COMMA; }
{HOST_PORT}     { strcpy(yylval.word, yytext); write(logfile, yytext, yyleng); return HOST_PORT; }
{NEWLINE}       { write(logfile, yytext, yyleng); return NEWLINE; }
.               ;

%%

