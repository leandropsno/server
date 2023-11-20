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
    if (argc < 2) {
        printf("Uso: ./servidor IP_Address Port_Number\n");
        exit(1);
    }

    int socket = connect2Server(argv[1], argv[2]);
    logfile = open("io/log.txt", O_CREAT | O_RDWR | O_APPEND, 00700); 
    strcpy(webSpacePath, argv[1]);

    char requestMessage[MAX_CONT], responseMessage[MAX_CONT];
    build_GET_requisition(requestMessage);
    write(socket, requestMessage, strlen(requestMessage));
    int i, j;
    do {
        i = read(socket, responseMessage, 1024);
        // if (i < 0) {
        //     // FIX ME
        // }
        // // yy_scan_string(requestMessage);
        j = write(logfile, responseMessage, i);
        if (j < 0) {
            perror("error: ");
        }
    } while (i >= 1024);
    // yyparse();
    
    close(socket);
    close(logfile);
    return 0;
}