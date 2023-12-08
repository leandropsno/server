#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <poll.h>
#include <stdint.h>
#include <crypt.h>
#include "lists.h"
#include "http.h"
#include "ast.h"

char webSpacePath[20] = "../../webspace";
int logfile = 0;

// Abre um socket na porta especificada.
int connectSocket(char *port) {
    struct sockaddr_in client, server;	
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket()");
        exit(1);
    }

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

// Tratador de erros internos.
void errorHandler(int socket, const char *message) {
    Response resp = createResponse();
    resp.code = INTERNAL_ERROR;
    codeMsg(&resp);
    httpError(socket, &resp, message);
    // if (thread != 0) {
    //     printf("Thread %ld processou o request com resultado %d\n", thread, resp.code); fflush(stdout);
    //     pthread_mutex_lock(&mutex1);
    //     n_threads++;
    //     printf("Thread %ld terminada, restam %d\n", thread, n_threads); fflush(stdout);     
    //     pthread_mutex_unlock(&mutex1);
    //     pthread_exit(0);
    // }
    // else {
    //     printf("Thread mãe processou o request com resultado %d\n", resp.code); fflush(stdout);
    // }
}

void main(int argc, char **argv) {
    printf("%s\n", crypt(argv[1], "84"));
}