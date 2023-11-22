#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include "http.h"
#include "lists.h"
#include "http.tab.h"

#define MAX_CHLD 5

CommandNode* mainList = NULL;
char webSpacePath[50];
int connectionSocket, messageSocket;
int N = 0;

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
    int n, pid, state;
    struct sockaddr_in cliente;
    unsigned int msgLen, nameLen;
    
    fd_set connections;
    FD_ZERO(&connections);
    struct timeval timeout;
    long int tolerancia = 10;
    timeout.tv_sec = tolerancia;
    timeout.tv_usec = 0;

    do {
        nameLen = sizeof(cliente);
        messageSocket = accept(connectionSocket, (struct sockaddr *)&cliente, &nameLen);
        
        if (N < MAX_CHLD) {
            pid = fork();
        }
        else {
            wait(&state);
            N--;
        }

        if (pid < 0){
            perror("Error in fork()");
            exit(1);
        }
        else if (pid > 0) {
            N++;
            shutdown(messageSocket, SHUT_RDWR);
            continue;
        }
        else if (pid == 0) {
            FD_SET(messageSocket, &connections);
            n = select(messageSocket + 1, &connections, (fd_set *)0, (fd_set *)0, &timeout);
            
            if (n > 0 && FD_ISSET(messageSocket, &connections)) {
                msgLen = read(messageSocket, requestMessage, sizeof(requestMessage));
            }
            else if (n == 0)  {
                printf("Nenhuma requisição recebida em %ld segundos\n", tolerancia);
                exit(0);
            }
            else {
                perror("Error in select()");
                exit(1);
            }

            yy_scan_string(requestMessage);
            yyparse();

            shutdown(messageSocket, SHUT_RDWR);
            exit(0);
        }
    } while (msgLen == 0);

    shutdown(connectionSocket, SHUT_RDWR); 
    return 0;
}