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
int connectionSocket, logfile;
int n_threads;
struct pollfd connection;
long unsigned int timeout = 4000;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void intHandler() {
    pthread_mutex_destroy(&mutex);
    close(connectionSocket);
    exit(0);
}

void errorHandler(int socket, const char *func, const char *message, pthread_t thread) {
    if (func != NULL) perror(func);
    Response resp = createResponse();
    resp.code = INTERNAL_ERROR;
    codeMsg(&resp);
    httpError(socket, &resp, message);
    if (thread != 0) {
        printf("Thread %ld processou o request com resultado %d\n", thread, resp.code); fflush(stdout);
        pthread_mutex_lock(&mutex);
        n_threads++;
        printf("Thread %ld terminada, restam %d\n", thread, n_threads); fflush(stdout);     
        pthread_mutex_unlock(&mutex);
        pthread_exit(0);
    }
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
            printf("Thread %ld leu %d bytes da seguinte requisicao: %s\n", pthread_self(), msgLen, requestMessage); fflush(stdout);              
            CommandNode* mainList = NULL; 
            yy_scan_string(requestMessage);
            int error = yyparse(&mainList, &result, connection.fd);
            mainList = NULL;
            cleanupList(&mainList);
            if (error == 1) {
                printf("yyparse() error: bad input\n");
                errorHandler(connection.fd, NULL, "", pthread_self());
            }
            else if (error == 2) {
                printf("yyparse() error: memory exhaustion\n");
                errorHandler(connection.fd, NULL, "", pthread_self());
            }
            printf("Thread %ld processou o request com resultado %d\n", pthread_self(), result); fflush(stdout);
        }
    }
    else if (n == 0)  {
        printf("Thread %ld não recebeu nenhuma requisição em %ld segundos\n", pthread_self(), timeout); fflush(stdout);
    }
    else if (n < 0) errorHandler(connection.fd, "Error in poll()", "", (pthread_t)0);
    pthread_mutex_lock(&mutex);
    shutdown(connection.fd, SHUT_RD);
    n_threads++;
    printf("Thread %ld terminada, restam %d\n", pthread_self(), n_threads); fflush(stdout);
    pthread_mutex_unlock(&mutex);
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

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Error in bind()");
        exit(1);
    }
    listen(sock, 5);
    printf("%s aceitando conexões\n", port);
    return sock;
}

void main(int argc, char **argv) {
    struct sockaddr_in cliente;
    unsigned int nameLen = sizeof(cliente);
    int msgSocket;

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

    signal(SIGINT, intHandler);

    // Realiza a conexão na porta especificada
    connectionSocket = connectSocket(argv[4]);
    while (1) {
        // Aguarda conexão no socket
        msgSocket = accept(connectionSocket, (struct sockaddr *)&cliente, &nameLen);
        if (msgSocket < 0 && errno == EINTR) continue;

        printf("Nova conexão recebida\n");

        // Cria thread, se houver alguma disponível.
        pthread_mutex_lock(&mutex);
        if (n_threads > 0) {
            n_threads--;
            pthread_mutex_unlock(&mutex);
            pthread_t thread;
            if (pthread_create(&thread, NULL, threadMain, (void *)(intptr_t)msgSocket) != 0) {
                errorHandler(msgSocket, "Error in phtread_create()", "", (pthread_t)0);
            }
        }
        else {
            errorHandler(msgSocket, NULL, "Servidor sobrecarregado. Tente novamente mais tarde.", (pthread_t)0);
            printf("Servidor sobrecarregado\n");
        }
    }
    pthread_mutex_destroy(&mutex);
    close(connectionSocket);
}