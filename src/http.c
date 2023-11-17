#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "http.h"

#define MAX_REQ 1024
#define MAX_CONT 4096
#define MAX_NAME 128
#define MAX_CMD 8
#define NOT_FOUND 404
#define FORBIDDEN 403
#define OK 200
#define INT_ERROR 500

extern FILE *resposta, *registro;

Response createResponse() {
    Response resp;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    strcpy(resp.rdate, asctime(localtime(&tv.tv_sec)));

    strcpy(resp.server, "Servidor HTTP versão 1.0 de Leandro Ponsano");

    strcpy(resp.connection, "keep-alive");

    return resp;
}

void codeMsg(char *msg, int code) {
    switch (code) {
        case OK:
            strcpy(msg, "OK");
            break;
        case NOT_FOUND:
            strcpy(msg, "Not Found");
            break;
        case FORBIDDEN:
            strcpy(msg, "Forbidden");
            break;
        case INT_ERROR:
            strcpy(msg, "Internal Server Error");
            break;
    }
}

int readContent(char *path, char *buf) {
    int fd;
    if ((fd = open(path, O_RDONLY)) == -1) {
        printf("Falha na abertura de arquivo\n");
        exit(INT_ERROR); 
    }
    ssize_t i;
    i = read(fd, buf, MAX_CONT);
    close(fd);
    return i;
}

void searchDir(char *path, Response *resp) {
    char resources[2][13] = {"/index.html", "/welcome.html"};
    char filename[MAX_NAME];
    int found = 0, read = 0;

    for (int i = 0; i < 2; i++) {
        // Monta path para o recurso padrão
        strcpy(filename, path);
        strcat(filename, resources[i]);

        // Acessa estatísticas do recurso
        struct stat file_stats;
        if (stat(filename, &file_stats) == 0) {
            found = 1;
            if (access(filename, R_OK) == 0) {
                // Encontrou um arquivo com permissão de leitura
                readContent(filename, resp->content);
                read = 1;
                resp->code = OK;
            }
        }
    }

    if (!found) { // Se nenhum dos dois foi encontrado
        resp->code = NOT_FOUND;
    }
    else if (!read) {  // Se algum foi encontrado mas nenhum tinha permissao de leitura
        resp->code = FORBIDDEN;
    }
}

void accessResource(char *path, Response *resp) {

    struct stat resource_stats;
    if (stat(path, &resource_stats) == -1) {    // Se o recurso não for encontrado
        resp->code = NOT_FOUND;
        return;
    }

    // Preenche Content-Length e Last-Modified
    resp->size = (int)resource_stats.st_size;
    strcpy(resp->lmdate, asctime(localtime(&resource_stats.st_mtime)));

    if ((access(path, R_OK) != 0)) {    // Se o recurso não possui permissão de leitura
        resp->code = FORBIDDEN;
        return;
    }

    switch (resource_stats.st_mode & S_IFMT)
    {
        case S_IFREG :  // Se o recurso for um arquivo regular
            readContent(path, resp->content);
            resp->code = OK;
            break;
        case S_IFDIR :  // Se o recurso for um diretório
            if ((access(path, X_OK) != 0)) {    // Se não tem permissão de varredura
                resp->code = FORBIDDEN;
            }
            else {
                searchDir(path, resp);
            }
            break;
    }
}

void GET(char *path, Response *resp) {
    accessResource(path, resp);
    codeMsg(resp->result, resp->code);
    fprintf(resposta, "HTTP/version %d %s\n", resp->code, resp->result);    //FIX VERSION
    fprintf(registro, "HTTP/version %d %s\n", resp->code, resp->result);    //FIX VERSION
    fprintf(resposta, "Date: %s", resp->rdate);
    fprintf(registro, "Date: %s", resp->rdate);
    fprintf(resposta, "Server: %s\n", resp->server);
    fprintf(registro, "Server: %s\n", resp->server);
    fprintf(resposta, "Connection: %s\n", resp->connection);
    fprintf(registro, "Connection: %s\n", resp->connection);
    fprintf(resposta, "Last-Modified: %s", resp->lmdate);
    fprintf(registro, "Last-Modified: %s", resp->lmdate);
    fprintf(resposta, "Content-Length: %d\n", resp->size);
    fprintf(registro, "Content-Length: %d\n", resp->size);
    fprintf(resposta, "Content-Type: ??????\n");
    fprintf(registro, "Content-Type: ??????\n");
    fprintf(resposta, "\n");
    fprintf(registro, "\n");
    if (resp->code == 200) {
        fprintf(resposta, "%s\n", resp->content);
    } 
}

void OPTIONS(char *path, Response *resp) {
    // codeMsg(resp->result, resp->code);
    // printf("HTTP/version %d %s\n", resp->code, resp->result);
    // printf("Allow: GET, OPTIONS, TRACE\n");
    // printf("Date: %s", resp->rdate);
    // printf("Server: %s\n", resp->server);
    // printf("Connection: %s\n", resp->connection);
    // printf("Content-Length: %d\n", resp->len);
    // printf("Content-Type: ??????\n");
    printf("CABECALHO DE RESPOSTA OPTIONS");
}

void TRACE(char *path, Response *resp) {
    // codeMsg(resp->result, resp->code);
    // printf("HTTP/version %d %s\n", resp->code, resp->result);
    // printf("Allow: GET, OPTIONS, TRACE\n");
    // printf("Date: %s", resp->rdate);
    // printf("Server: %s\n", resp->server);
    // printf("Connection: %s\n", resp->connection);
    // printf("Content-Length: %d\n", resp->len);
    // printf("Content-Type: ??????\n");
    printf("CABECALHO DE RESPOSTA TRACE");
}

int processRequisition(char *host, char *resource) {
    // Monta o path para o recurso
    char path[MAX_NAME] = "";
    strcat(path, host);
    strcat(path, "/");
    strcat(path, resource);

    Response resp = createResponse();
    GET(path, &resp);

    return 0;
}

