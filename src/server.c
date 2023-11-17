#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "http.h"
#include "lists.h"

CommandNode* mainList = NULL;
extern FILE *yyin;
FILE *registro, *resposta;
char webSpacePath[50];

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Uso: ./servidor <WebSpace> req_N.txt resp_N.txt registro.txt\n");
        exit(1);
    }
    yyin = fopen(argv[2], "r");  // Abre o arquivo de entrada req_N.txt
    resposta = fopen(argv[3], "w+"); // Abre o arquivo de saída resp_N.txt
    registro = fopen(argv[4], "a"); // Abre o arquivo de registro registro.txt
    strcpy(webSpacePath, argv[1]);
    yyparse();
    fclose(yyin);
    fclose(resposta);
    fclose(registro);
    return 0;
}