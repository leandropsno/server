#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "http.h"
#include "lists.h"
#include "http.tab.h"

CommandNode* mainList = NULL;
char webSpacePath[50];
int connectionSocket, messageSocket;

int main(int argc, char **argv) {

    if (argc < 3) {
        printf("Uso: ./servidor <web_space_path> <port_number>\n");
        exit(1);
    }
    strcpy(webSpacePath, argv[1]);

    connectionSocket = connectSocket(argv[2]);
    if (socket > 0) {
        printf("%s aceitando conexões\n", argv[2]);
    }
    else {
        perror("Error in connection");
        exit(1);
    }

    char requestMessage[MAX_CONT], responseMessage[MAX_CONT];
    int i, j;
    struct sockaddr_in cliente;
    unsigned int msgLen, nameLen;

    do {
        nameLen = sizeof(cliente);
        messageSocket = accept(connectionSocket, (struct sockaddr *)&cliente, &nameLen);

        msgLen = read(messageSocket, requestMessage, sizeof(requestMessage));
        
        yy_scan_string(requestMessage);
        yyparse();

        shutdown(messageSocket, SHUT_RDWR);
    } while (msgLen == 0);

    shutdown(connectionSocket, SHUT_RDWR); 
    return 0;
}