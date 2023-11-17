#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http.h"
#include "lists.h"

CommandNode* mainList = NULL;
extern FILE *yyin;
FILE *logfile, *respfile;
char webSpacePath[50];

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Uso: ./servidor <WebSpace> req_N.txt resp_N.txt logfile.txt\n");
        exit(1);
    }
    yyin = fopen(argv[2], "r");  // Abre o arquivo de entrada req_N.txt
    respfile = fopen(argv[3], "w+"); // Abre o arquivo de saída resp_N.txt
    logfile = fopen(argv[4], "a"); // Abre o arquivo de logfile logfile.txt
    strcpy(webSpacePath, argv[1]);
    yyparse();
    fclose(yyin);
    fclose(respfile);
    fclose(logfile);
    return 0;
}