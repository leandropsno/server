#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include "lists.h"
#include "http.h"

#define MAX_REQ 1024
#define MAX_NAME 128
#define MAX_CMD 8
#define NOT_FOUND 404
#define FORBIDDEN 403
#define OK 200
#define INTERNAL_ERROR 500
#define PRINT_CONT_HEADER 1
#define PRINT_LM 2
#define PRINT_ALLOW 4
#define PRINT_CONTENT 8

extern int logfile;
extern char webSpacePath[50];

void httpError(int socket, Response *resp, const char *message) {
    sprintf(resp->content, "<!DOCTYPE html>\n<html>\n\t<head>\n\t\t<title>%d</title>\n\t</head>\n\t<body>\n\t\t<h1>ERROR %d</h1>\n\t\t<p>%s<br>%s.</p>\n\t</body>\n</html>", resp->code, resp->code, resp->result, message);
    resp->size = strlen(resp->content);
    flushResponse(socket, resp, PRINT_CONT_HEADER | PRINT_CONTENT);
}

Response createResponse() {
    Response resp;
    struct timeval tv;
    gettimeofday(&tv, NULL);

    strcpy(resp.rdate, asctime(localtime(&tv.tv_sec)));
    resp.rdate[strlen(resp.rdate)-1] = 0;   // Tira quebra de linha do final
    strcpy(resp.server, "Servidor HTTP versão 11 de Leandro Ponsano");
    strcpy(resp.connection, "close");
    strcpy(resp.allow, "GET, HEAD, OPTIONS, TRACE");
    resp.size = 0;
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
        case INTERNAL_ERROR:
            strcpy(resp->result, "Internal Server Error");
            break;
    }
}

void getType(char *type, char *filename) {
    char name[strlen(filename)];
    strcpy(name, filename);
    char *tok = strtok(name, ".");
    while (tok != NULL) {
        strcpy(type, tok);
        tok = strtok(NULL, ".");
    }
}

int readContent(char *path, Response *resp) {
    int fd;
    if ((fd = open(path, O_RDONLY)) == -1) {
        resp->code = INTERNAL_ERROR;
        return -1; 
    }
    ssize_t i;
    i = read(fd, resp->content, MAX_CONT);
    close(fd);
    return i;
}

int checkPath(char *path) {
    char str[strlen(path)];
    strcpy(str, path);
    int counter = 0;
    char *dir = strtok(str, "/");
    while (dir != NULL) {
        if (strcmp(dir, "..") == 0) {
            counter--;
        }
        else if ((strcmp(dir, "") == 0) || (strcmp(dir, ".") == 0));
        else {
            counter++;
        }
        if (counter < 0) {
            break;
        }
        dir = strtok(NULL, "/");
    }
    return counter;
}

void searchDir(char *path, Response *resp) {
    char resources[2][13] = {"/index.html", "/welcome.html"};
    char filename[MAX_NAME];
    int found = 0, read = 0, len;

    for (int i = 0; i < 2; i++) {
        // Monta path para o recurso padrão
        strcpy(filename, path);
        strcat(filename, resources[i]);

        // Acessa estatísticas do recurso
        struct stat file_stats;
        if (stat(filename, &file_stats) == 0) {
            found = 1;
            if (access(filename, R_OK) == 0) { // Encontrou um arquivo com permissão de leitura
                len = readContent(filename, resp);
                resp->code = OK;  
                read = 1;
                resp->size = len;
                strcpy(resp->type, "text/html");
                strcpy(resp->lmdate, asctime(localtime(&file_stats.st_mtime)));
                resp->lmdate[strlen(resp->lmdate)-1] = 0;   // Tira quebra de linha do final
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

    switch (resource_stats.st_mode & S_IFMT)
    {
        case S_IFREG :  // Se o recurso for um arquivo regular
            
            if ((access(path, R_OK) != 0)) {    // Se não tem permissão de leitura
                resp->code = FORBIDDEN;
            }
            else {
                // Preenche Content-Type, Content-Length e Last-Modified
                getType(resp->type, path);
                int len = readContent(path, resp);
                resp->code = OK;
                resp->size = len;
                strcpy(resp->lmdate, asctime(localtime(&resource_stats.st_mtime)));
                resp->lmdate[strlen(resp->lmdate)-1] = 0;   // Tira quebra de linha do final
            }
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

void flushResponse(int fd, Response *resp, int fields) {
    dprintf(fd, "HTTP/1.1 %d %s\r\n", resp->code, resp->result);  
    dprintf(fd, "Date: %s\r\n", resp->rdate);
    dprintf(fd, "Server: %s\r\n", resp->server);    
    dprintf(fd, "Connection: %s\r\n", resp->connection);
    if (fields & PRINT_CONT_HEADER) { 
        dprintf(fd, "Content-Type: %s\r\n", resp->type);
        dprintf(fd, "Content-Length: %d\r\n", resp->size);   
    }
    if (fields & PRINT_ALLOW) {
        dprintf(fd, "Allow: %s\r\n", resp->allow);
    }
    if (fields & PRINT_LM) {
        dprintf(fd, "Last-Modified: %s\r\n", resp->lmdate);  
    }
    if (fields & PRINT_CONTENT) {
        dprintf(fd, "\r\n");
        write(fd, resp->content, resp->size);
    }
    dprintf(fd, "\r\n");
}

void GET(char *path, Response *resp, int socket) {
    accessResource(path, resp);
    codeMsg(resp);
    if (resp->code == 200) {
        flushResponse(socket, resp, PRINT_CONT_HEADER | PRINT_CONTENT | PRINT_LM);
    }
    else {
        httpError(socket, resp, "");
    }
}

void HEAD(char *path, Response *resp, int socket) {
    accessResource(path, resp);
    codeMsg(resp);
    if (resp->code == 200) {
        flushResponse(socket, resp, PRINT_CONT_HEADER | PRINT_LM);
    }
    else {
        httpError(socket, resp, "");
    }
}

void OPTIONS(char *path, Response *resp, int socket) {
    accessResource(path, resp);
    codeMsg(resp);
    if (resp->code == 200) {
        flushResponse(socket, resp, PRINT_ALLOW);
    }
    else {
        httpError(socket, resp, "");
    }
}

void TRACE(char *path, Response *resp, int socket) {
    resp->code = OK;
    codeMsg(resp);
    strcpy(resp->type, "message/html");
    flushResponse(socket, resp, PRINT_CONT_HEADER | PRINT_CONTENT);
}

int processRequest(listptr mainList, int socket) {
    Response resp = createResponse();
    
    CommandNode *list = *mainList;
    char *method = list->command;
    char *resource = list->paramList->parameter;

    // Se o recurso está fora do webspace
    if (checkPath(&resource[1]) < 0) {
        return FORBIDDEN;
    }

    // Monta o path para o recurso
    char path[MAX_NAME] = "";
    strcat(path, webSpacePath);
    strcat(path, resource);
    
    if (strcmp(method, "GET") == 0) {
       GET(path, &resp, socket); 
    }
    else if (strcmp(method, "HEAD") == 0) {
        HEAD(path, &resp, socket);
    }
    else if (strcmp(method, "TRACE") == 0) {
        printOriginal(resp.content, mainList);
        TRACE(path, &resp, socket);
    }
    else if (strcmp(method, "OPTIONS") == 0) {
        OPTIONS(path, &resp, socket);
    }

    return resp.code;
}
