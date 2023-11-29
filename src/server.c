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
#include <sys/resource.h>
#include <netinet/in.h>
#include <signal.h>
#include <poll.h>
#include "http.h"
#include "lists.h"
#include "http.tab.h"

CommandNode* mainList = NULL;
char webSpacePath[50];
int connectionSocket, messageSocket, logfile;
int N = 0;
int MAX_CHLD;

int main(int argc, char **argv) {

    // Verifica número de argumentos
    if (argc < 5) {
        printf("Uso: ./servidor <web_space_path> <port_number> <max_child> <logfile>\n");
        exit(1);
    }

    // Seta o número máximo de processos-filho
    MAX_CHLD = atoi(argv[3]);

    logfile = open(argv[4], O_CREAT | O_APPEND | O_RDWR, 00700);

    // Seta o caminho do webspace
    strcpy(webSpacePath, argv[1]);

    // Realiza a conexão na porta especificada
    connectionSocket = connectSocket(argv[2]);

    // Define a rotina de tratamento do sinal SIGCHLD
    void handler();
    signal(SIGCHLD, handler);

    char requestMessage[MAX_CONT];
    int n, pid, child = 0, state;
    struct sockaddr_in cliente;
    unsigned int msgLen, nameLen = sizeof(cliente);
   
    struct pollfd connection;
    connection.fd = 0;
    connection.events = POLLIN;
    long int timeout = 2000;
    
    printf("PAI: %d\n", getpid()); fflush(stdout);

    while (1) {
        messageSocket = accept(connectionSocket, (struct sockaddr *)&cliente, &nameLen);
        if (N < MAX_CHLD) {
            printf("Pai ainda tem %d filhos disponiveis\n", MAX_CHLD - N); fflush(stdout);
            N++;
            pid = fork(); 
            if (pid < 0) {
                perror("Error in fork()");
                exit(1);
            }
            else if (pid == 0) {
                printf("FILHO: %d\n", getpid()); fflush(stdout);
                
                connection.fd = messageSocket;
                n = poll(&connection, 1, timeout);
                printf("Filho %d reconheceu %d sockets prontos pra leitura \n", getpid(), n); fflush(stdout);
                if (n > 0 && connection.revents == POLLIN) {
                    msgLen = read(messageSocket, requestMessage, sizeof(requestMessage));
                    printf("Filho %d leu %d bytes de requisicao\n", getpid(), msgLen); fflush(stdout);              
                    yy_scan_string(requestMessage);
                    yyparse();
                    close(connection.fd);
                    exit(0);      
                }
                else if (n == 0)  {
                    printf("Filho %d não recebeu nenhuma requisição em %ld segundos\n", getpid(), timeout); fflush(stdout);
                }
                else {
                    perror("Error in poll()");
                    exit(1);
                }
            }
            else {
                // do something
            }
        }
        else {
            printf("Servidor sobrecarregado\n"); fflush(stdout);
        }
    }
    shutdown(connectionSocket, SHUT_RDWR);
    printf("Pai %d encerrou\n", getpid());
    return 0;
}

void handler() {
    int pid;
    int estado;

    pid = wait3(&estado, WNOHANG, NULL);
    N--;
    printf("Filho %d encerrou\n", pid); fflush(stdout); 
    printf("Pai ainda tem %d filhos disponiveis\n", MAX_CHLD - N); fflush(stdout);         
}

