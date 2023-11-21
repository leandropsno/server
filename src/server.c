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

CommandNode* mainList = NULL;
int logfile;
char webSpacePath[50];

void build_GET_requisition(char *req) {
    strcpy(req, "GET / HTTP/1.1\r\nHost: example.org\r\n\r\n");
}

int main(int argc, char **argv) {

    if (argc < 1) {
        printf("Uso: ./servidor Port_Number\n");
        exit(1);
    }
    
    logfile = open("io/log.txt", O_CREAT | O_RDWR | O_APPEND, 00700); 
    strcpy(webSpacePath, argv[1]);

    int socket = connectSocket(argv[1]);
    if (socket > 0) {
        printf("%s aceitando conexões\n", argv[1]);
    }
    else {
        perror("Error in connection");
        exit(1);
    }

    char requestMessage[MAX_CONT], responseMessage[MAX_CONT];
    int i, j;
    struct sockaddr_in cliente;
    unsigned int msgLen, nameLen;
    int newSocket;

    while (1) {
        nameLen = sizeof(cliente);
        newSocket = accept(socket, (struct sockaddr *)&cliente, &nameLen);

        msgLen = read(newSocket, requestMessage, sizeof(requestMessage));
        
        yy_scan_string(requestMessage);
        yyparse();
        yy_delete_buffer(YY_CURRENT_BUFFER);

        printf("\nMensagem recebida:\n");
        for(i = 0; i < msgLen; i++) printf("%c", requestMessage[i]);
        fflush(stdout);
        shutdown(newSocket, SHUT_RDWR);
    }

    shutdown(socket, SHUT_RDWR); 
    close(logfile);
    return 0;
}