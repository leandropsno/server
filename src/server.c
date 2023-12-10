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
long unsigned int timeout = 1000;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

// Tratador de Interrupção
void intHandler() {
    printf("\nEncerrando.\n");
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    close(connectionSocket);
    exit(0);
}

// Tratador de erros internos.
void errorHandler(int socket, const char *func, const char *message, pthread_t thread) {
    if (func != NULL) perror(func);
    Response resp = createResponse();
    resp.code = INTERNAL_ERROR;
    codeMsg(&resp);
    httpError(socket, &resp, message);
    close(socket);
    if (thread != 0) {
        pthread_mutex_lock(&mutex1);
        n_threads++;
        pthread_mutex_unlock(&mutex1);
        pthread_exit(0);
    }
}

// Função principal das threads. Processa a requisição (se houver) em SOCKET.
void *threadMain(void *socket) {
    int result, msgLen, error;
    struct pollfd connection;
    char requestMessage[MAX_CONT];

    connection.events = POLLIN;
    connection.fd = (intptr_t)socket;
    int n = poll(&connection, 1, timeout);

    if ((n > 0) && (connection.revents == POLLIN)) {
        msgLen = read(connection.fd, requestMessage, MAX_CONT);
        if (msgLen > 0) {
            requestMessage[msgLen] = 0;         
            CommandNode* mainList = NULL;
            pthread_mutex_lock(&mutex2);
            write(logfile, "----- Novo Par Requisição/Resposta -----\n", 42);
            yy_scan_string(requestMessage);
            error = yyparse(&mainList, &result, connection.fd);
            write(logfile, "----------------------------------------\n\n", 43);
            pthread_mutex_unlock(&mutex2);
            mainList = NULL;
            cleanupList(&mainList);
            if (error == 1) {
                errorHandler(connection.fd, NULL, "", pthread_self());
            }
            else if (error == 2) {
                errorHandler(connection.fd, "Error in yyparse()", "", pthread_self());
            }
        }
    }
    else if (n < 0) {
        errorHandler(connection.fd, "Error in poll()", "", (pthread_t)0);
    }
    pthread_mutex_lock(&mutex1);
    n_threads++;
    close(connection.fd);
    pthread_mutex_unlock(&mutex1);
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
    if ((argc < 4) || (!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help"))) {
        printf("Uso: ./servidor <web_space> <N> <port_number> <log_file>\n");
        exit(1);
    }
    
    // Define o tratador de interrupção
    signal(SIGINT, intHandler);

    // Seta o caminho do webspace
    strcpy(webSpacePath, argv[1]);

    // Seta o número máximo de threads
    n_threads = atoi(argv[2]);
    if (n_threads <= 0) {
        printf("N deve ser maior que 0");
        exit(1);
    }
    
    // Realiza a conexão na porta especificada
    connectionSocket = connectSocket(argv[3]);
    // sleep(30);

    // Abre o arquivo de log (registro)
    logfile = open(argv[4], O_CREAT | O_APPEND | O_RDWR, 00700);
    
    while (1) {
        // Aguarda conexão no socket
        msgSocket = accept(connectionSocket, (struct sockaddr *)&cliente, &nameLen);
        if (msgSocket < 0 && errno == EINTR) continue;

        // Cria thread, se houver alguma disponível.
        pthread_mutex_lock(&mutex1);
        if (n_threads > 0) {
            n_threads--;
            pthread_mutex_unlock(&mutex1);
            pthread_t thread;
            if (pthread_create(&thread, NULL, threadMain, (void *)(intptr_t)msgSocket) != 0) {
                errorHandler(msgSocket, "Error in phtread_create()", "", (pthread_t)0);
            }
        }
        else {
            pthread_mutex_unlock(&mutex1);
            errorHandler(msgSocket, NULL, "Servidor sobrecarregado. Tente novamente mais tarde.", (pthread_t)0);
        }
        
    }
}