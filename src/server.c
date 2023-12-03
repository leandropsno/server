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
#include <pthread.h>
#include <stdint.h>
#include "lists.h"
#include "http.h"
#include "http.tab.h"

#define INTERNAL_ERROR 500

char webSpacePath[50];
int connectionSocket, messageSocket, logfile;
int n_threads;
struct pollfd connection;
long unsigned int timeout = 2000;

void errorHandler(const char *func, const char *message) {
    if (func != NULL) perror(func);
    Response resp = createResponse();
    resp.code = INTERNAL_ERROR;
    codeMsg(&resp);
    httpError(&resp, message);
    printf("Thread %ld processou o request com resultado %d\n", pthread_self(), resp.code); fflush(stdout);
    n_threads++;
    printf("Thread %ld terminada, restam %d\n", pthread_self(), n_threads); fflush(stdout);     
    pthread_exit(0);
}

void *threadMain(void *socket) {
    int result;
    printf("Thread %ld criada, restam %d\n", pthread_self(), n_threads); fflush(stdout);
    connection.events = POLLIN;
    connection.fd = (intptr_t)socket;
    int n = poll(&connection, 1, timeout);
    if ((n > 0) && (connection.revents == POLLIN)) {
        char requestMessage[MAX_CONT];
        int msgLen = read(connection.fd, requestMessage, sizeof(requestMessage));
        if (msgLen > 0) {
            printf("Thread %ld leu %d bytes de requisicao\n", pthread_self(), msgLen); fflush(stdout);              
            listptr mainList = (listptr)malloc(sizeof(CommandNode *));
            yy_scan_string(requestMessage);
            yyparse(mainList, &result);
            cleanupList(mainList);
            printf("Thread %ld processou o request com resultado %d\n", pthread_self(), result); fflush(stdout);
        }
    }
    else if (n == 0)  {
        printf("Thread %ld não recebeu nenhuma requisição em %ld segundos\n", pthread_self(), timeout); fflush(stdout);
    }
    else if (n < 0) errorHandler("Error in poll()", NULL);
    shutdown(connection.fd, SHUT_RD);
    n_threads++;
    printf("Thread %ld terminada, restam %d\n", pthread_self(), n_threads); fflush(stdout);
    pthread_exit(0);
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
    struct sockaddr_in cliente;
    unsigned int nameLen = sizeof(cliente);

    // Verifica número de argumentos
    if ((argc < 5) || (!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help"))) {
        printf("Uso: ./servidor <web_space_path> <logfile> <n_threads> <port_number>\n");
        exit(1);
    }

    // Seta o caminho do webspace
    strcpy(webSpacePath, argv[1]);

    // Abre o arquivo de log (registro)
    logfile = open(argv[2], O_CREAT | O_APPEND | O_RDWR, 00700);

    // Seta o número máximo de threads
    n_threads = atoi(argv[3]);
    if (n_threads <= 0) {
        printf("n_threads deve ser maior que 0");
        exit(1);
    }

    // Realiza a conexão na porta especificada
    connectionSocket = connectSocket(argv[4]);

    while (1) {
        // Aguarda conexão no socket
        messageSocket = accept(connectionSocket, (struct sockaddr *)&cliente, &nameLen);
        if (messageSocket < 0 && errno == EINTR) continue;

        // Cria thread, se houver alguma disponível.
        if (n_threads > 0) {
            n_threads--;
            pthread_t thread;
            if (pthread_create(&thread, NULL, threadMain, (void *)&messageSocket) != 0) errorHandler("Error in phtread_create()", NULL);
        }
        else errorHandler(NULL, "Servidor sobrecarregado. Tente novamente mais tarde.");
    }
    shutdown(connectionSocket, SHUT_RDWR);
    return 0;
}