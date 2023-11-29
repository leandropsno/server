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

    char requestMessage[MAX_CONT];
    int n, pid, child = 0, state;
    struct sockaddr_in cliente;
    unsigned int msgLen, nameLen = sizeof(cliente);
   
    fd_set connections;
    FD_ZERO(&connections);
    struct timeval timeout;
    long int tolerancia = 1;
    timeout.tv_sec = tolerancia;
    timeout.tv_usec = 0;
    
    // Define a rotina de tratamento do sinal SIGCHLD
    void ger_sinal();
    signal(SIGCHLD, ger_sinal);

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
                sleep(7);
                printf("FILHO: %d\n", getpid()); fflush(stdout);

                FD_SET(messageSocket, &connections);
                n = select(messageSocket + 1, &connections, (fd_set *)0, (fd_set *)0, &timeout);
                printf("Filho %d reconheceu %d sockets prontos pra leitura \n", getpid(), n); fflush(stdout);
                if (n > 0 && FD_ISSET(messageSocket, &connections)) {
                    msgLen = read(messageSocket, requestMessage, sizeof(requestMessage));
                    printf("Filho %d leu %d bytes da segunte requisicao:\n\n\"%s\"\n\n\n", getpid(), msgLen, requestMessage); fflush(stdout);              
                    yy_scan_string(requestMessage);
                    yyparse();
                    exit(0);      
                }
                else if (n == 0)  {
                    printf("Filho %d não recebeu nenhuma requisição em %ld segundos\n", getpid(), tolerancia); fflush(stdout);
                    exit(0);
                }
                else {
                    perror("Error in select()");
                    exit(1);
                }
            }
            else {
                // sleep(2);
                // shutdown(messageSocket, SHUT_RDWR);
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

void ger_sinal() {
    int pid;
    int estado;

    pid = wait3(&estado, WNOHANG, NULL);
    N--;
    printf("Filho %d encerrou\n", pid); fflush(stdout); 
    printf("Pai ainda tem %d filhos disponiveis\n", MAX_CHLD - N); fflush(stdout);         
}