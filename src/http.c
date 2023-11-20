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
#include "lists.h"

#define MAX_REQ 1024
#define MAX_NAME 128
#define MAX_CMD 8
#define NOT_FOUND 404
#define FORBIDDEN 403
#define OK 200
#define INT_ERROR 500

extern FILE *respfile, *logfile;
extern CommandNode *mainList;

void httpError(Response *resp) {
    sprintf(resp->content, "<!DOCTYPE html>\n<html>\n\t<head>\n\t\t<title>%d</title>\n\t</head>\n\t<body>\n\t\t<h1>ERROR %d</h1>\n\t\t<p>%s.</p>\n\t</body>\n</html>", resp->code, resp->code, resp->result);
}

Response createResponse() {
    Response resp;
    struct timeval tv;
    gettimeofday(&tv, NULL);

    strcpy(resp.rdate, asctime(localtime(&tv.tv_sec)));
    strcpy(resp.server, "Servidor HTTP versão 6 de Leandro Ponsano");
    strcpy(resp.connection, "keep-alive");
    strcpy(resp.allow, "GET, HEAD, OPTIONS, TRACE");

    return resp;
}

void codeMsg(Response *resp) {
    switch (resp->code) {
        case OK:
            strcpy(resp->result, "OK");
            break;
        case NOT_FOUND:
            strcpy(resp->result, "Not Found");
            break;
        case FORBIDDEN:
            strcpy(resp->result, "Forbidden");
            break;
        case INT_ERROR:
            strcpy(resp->result, "Internal Server Error");
            break;
    }
}

int readContent(char *path, Response *resp) {
    int fd;
    if ((fd = open(path, O_RDONLY)) == -1) {
        resp->code = INT_ERROR;
        return -1; 
    }
    ssize_t i;
    i = read(fd, resp->content, MAX_CONT);
    close(fd);
    resp->code = OK;
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
            strcpy(resp->type, "text/html");
            if (access(filename, R_OK) == 0) {
                // Encontrou um arquivo com permissão de leitura
                readContent(filename, resp);
                read = 1;
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
            strcpy(resp->type, "text/html");
            readContent(path, resp);
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

void flushCommonHeader(Response *resp) {
    fprintf(respfile, "HTTP/1.1 %d %s\n", resp->code, resp->result);    
    fprintf(logfile, "HTTP/1.1 %d %s\n", resp->code, resp->result);    
    fprintf(respfile, "Date: %s", resp->rdate);
    fprintf(logfile, "Date: %s", resp->rdate);
    fprintf(respfile, "Server: %s\n", resp->server);
    fprintf(logfile, "Server: %s\n", resp->server);
    fprintf(respfile, "Connection: %s\n", resp->connection);
    fprintf(logfile, "Connection: %s\n", resp->connection);
}

void flushContentHeaders(Response *resp) {
    fprintf(respfile, "Last-Modified: %s", resp->lmdate);
    fprintf(logfile, "Last-Modified: %s", resp->lmdate);
    fprintf(respfile, "Content-Length: %d\n", resp->size);
    fprintf(logfile, "Content-Length: %d\n", resp->size);
    fprintf(respfile, "Content-Type: %s\n", resp->type);
    fprintf(logfile, "Content-Type: %s\n", resp->type);
}

void flushContent(Response *resp) {
    fprintf(respfile, "\n");
    fprintf(logfile, "\n");
    fprintf(respfile, "%s\n", resp->content);
}

void GET(char *path, Response *resp) {
    accessResource(path, resp);
    codeMsg(resp);
    flushCommonHeader(resp);
    if (resp->code == 200) {
        flushContentHeaders(resp);
    }
    else {
        httpError(resp);
    }
    flushContent(resp);
}

void HEAD(char *path, Response *resp) {
    accessResource(path, resp);
    codeMsg(resp);
    flushCommonHeader(resp);
    if (resp->code == 200) {
        flushContentHeaders(resp);
    }
    else {
        httpError(resp);
    }
}

void OPTIONS(char *path, Response *resp) {
    accessResource(path, resp);
    codeMsg(resp);
    fprintf(respfile, "Allow: %s\n", resp->allow);
    fprintf(logfile, "Allow: %s\n", resp->allow);
    flushCommonHeader(resp);
    fprintf(respfile, "\n");
    fprintf(logfile, "\n");
}

void TRACE(char *path, Response *resp) {
    resp->code = OK;
    codeMsg(resp);
    printOriginal(resp->content, mainList);
    strcpy(resp->type, "message/html");
    fprintf(respfile, "Content-Type: %s\n", resp->type);
    fprintf(logfile, "Content-Type: %s\n", resp->type);
    flushContent(resp);
}

int processRequisition(char *method, char *host, char *resource) {
    // Monta o path para o recurso
    char path[MAX_NAME] = "";
    strcat(path, host);
    strcat(path, "/");
    strcat(path, resource);

    Response resp = createResponse();
    
    if (strcmp(method, "GET") == 0) {
       GET(path, &resp); 
    }
    else if (strcmp(method, "HEAD") == 0) {
        HEAD(path, &resp);
    }
    else if (strcmp(method, "TRACE") == 0) {
        TRACE(path, &resp);
    }
    else if (strcmp(method, "OPTIONS") == 0) {
        OPTIONS(path, &resp);
    }

    return 0;
}