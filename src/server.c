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

#define INTERNAL_ERROR 500

CommandNode* mainList = NULL;
char webSpacePath[50];
int connectionSocket, messageSocket, logfile;
int N = 0;
int MAX_CHLD;

void childHandler() {
    int pid, estado;
    pid = wait3(&estado, WNOHANG, NULL);
    do {
        N--;
        printf("Filho %d encerrado: ainda restam %d\n", pid, MAX_CHLD - N); fflush(stdout);  
        pid = wait3(&estado, WNOHANG, NULL);
    } while (pid > 0);       
}

void errorHandler(const char *func, char *message) {
    if (func != NULL) perror(func);
    Response resp = createResponse();
    resp.code = INTERNAL_ERROR;
    codeMsg(&resp);
    httpError(&resp, message);
}

// Abre um socket na porta especificada.
int connectSocket(char *port) {
    struct sockaddr_in client, server;	
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons((unsigned short)atoi(port));
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) errorHandler("Error in bind()", NULL);
    listen(sock, 5);
    printf("%s aceitando conexões\n", port);
    return sock;
}

int main(int argc, char **argv) {
    char requestMessage[MAX_CONT];
    int n, pid, child = 0;
    struct sockaddr_in cliente;
    unsigned int msgLen, nameLen = sizeof(cliente);
    struct pollfd connection; connection.events = POLLIN;
    long int timeout = 2000;

    // Verifica número de argumentos
    if (argc < 5) {
        printf("Uso: ./servidor <web_space_path> <port_number> <max_child> <logfile>\n");
        exit(1);
    }

    // Seta o número máximo de processos-filho
    MAX_CHLD = atoi(argv[3]);
    if (MAX_CHLD <= 0) {
        printf("max_child deve ser maior que 0");
        exit(1);
    }

    // Abre o arquivo de log (registro)
    logfile = open(argv[4], O_CREAT | O_APPEND | O_RDWR, 00700);

    // Seta o caminho do webspace
    strcpy(webSpacePath, argv[1]);

    // Realiza a conexão na porta especificada
    connectionSocket = connectSocket(argv[2]);

    // Define as rotinas de tratamento de sinais
    void childHandler();
    signal(SIGCHLD, childHandler);
    sleep(60);

    while (1) {
        messageSocket = accept(connectionSocket, (struct sockaddr *)&cliente, &nameLen);
        if (messageSocket < 0 && errno == EINTR) continue;
        printf("Recebi uma conexão\n");
        if (N < MAX_CHLD) {
            N++;
            pid = fork(); 
            if (pid == 0) {
                printf("Filho %d criado: ainda restam %d\n", getpid(), MAX_CHLD - N); fflush(stdout);
                connection.fd = messageSocket;
                n = poll(&connection, 1, timeout);
                if (n > 0 && connection.revents == POLLIN) {
                    msgLen = read(messageSocket, requestMessage, sizeof(requestMessage));
                    printf("Filho %d leu %d bytes de requisicao\n", getpid(), msgLen); fflush(stdout);              
                    yy_scan_string(requestMessage);
                    yyparse();
                    shutdown(connection.fd, SHUT_RD);
                    exit(0);      
                }
                else if (n == 0)  {
                    printf("Filho %d não recebeu nenhuma requisição em %ld segundos\n", getpid(), timeout); fflush(stdout);
                    shutdown(connection.fd, SHUT_RD);
                    exit(0);
                }
                else errorHandler("Error in poll()", NULL);
            }
            else if (pid > 0) {}
            else errorHandler("Error in fork()", NULL);
        }
        else errorHandler(NULL, "Servidor sobrecarregado. Tente novamente mais tarde.");
    }
    shutdown(connectionSocket, SHUT_RDWR);
    return 0;
}