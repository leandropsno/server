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
#include <pthread.h>
#include <crypt.h>
#include "lists.h"
#include "ast.h"
#include "http.h"

const char *fileTable[TABLE_SIZE][2] = {
    {"htm", "text/html"}, {"html", "text/html"}, {"txt", "text/plain"}, {"css", "text/css"}, {"csv", "text/csv"}, {"js", "text/javascript"},
    {"avif", "image/avif"}, {"bmp", "image/bmp"}, {"gif", "image/gif"}, {"jpg", "image/jpeg"},  {"jpeg", "image/jpeg"}, {"png", "image/png"}, {"tif", "image/tiff"}, {"tiff", "image/tiff"}, {"webp", "image/webp"},
    {"zip", "application/zip"}, {"gzip", "application/gzip"}, {"json", "application/json"},  {"pdf", "application/pdf"}, {"rar", "application/x-rar-compressed"}, {"tar", "application/x-tar"}, {"rtf", "application/rtf"}, {"sh", "application/x-sh"}, {"xhtml", "application/xhtml+xml"}, {"xls", "application/vnd.ms-excel"}, {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"}, {"xml", "application/xml"},
    {"mp3", "audio/mpeg"}, {"wav", "audio/wav"}, {"mp4", "video/mp4"}, {"webm", "video/webm"}
};

extern int logfile;
extern char webSpacePath[50];

Response createResponse() {
    Response resp;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    strcpy(resp.rdate, asctime(localtime(&tv.tv_sec)));
    resp.rdate[strlen(resp.rdate)-1] = 0;   // Tira quebra de linha do final
    strcpy(resp.server, "Servidor HTTP versão 13 de Leandro Ponsano");
    strcpy(resp.connection, "close");
    strcpy(resp.allow, "GET, HEAD, OPTIONS, TRACE");
    resp.size = 0;
    return resp;
}

void httpError(int socket, Response *resp, const char *message) {
    sprintf(resp->content, "<!DOCTYPE html>\n<html>\n\t<head>\n\t\t<title>%d</title>\n\t</head>\n\t<body>\n\t\t<h1>ERROR %d</h1>\n\t\t<p>%s<br>%s.</p>\n\t</body>\n</html>", resp->code, resp->code, resp->result, message);
    resp->size = strlen(resp->content);
    strcpy(resp->type, "text/html");
    int fields = PRINT_TYPE_LENGTH | PRINT_CONTENT;
    if (resp->code == AUTH_REQUIRED) fields |= PRINT_AUTH;
    flushResponse(socket, resp, fields);
    fields = PRINT_TYPE_LENGTH;
    if (resp->code == AUTH_REQUIRED) fields |= PRINT_AUTH;
    flushResponse(logfile, resp, fields);
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
        case AUTH_REQUIRED:
            strcpy(resp->result, "Authorization Required");
            break;
    }
}

void getMediaType(char *type, char *filename) {
    char extension[5];
    char tok[strlen(filename)];
    int i;

    // Extrai a extensão do nome do arquivo
    char *next = mystrtok(filename, tok, '.');
    while (next != NULL) {
        next = mystrtok(next, tok, '.');
    }
    strcpy(extension, tok);

    // Procura pela extensão na tabela de tipos
    for (i = 0; i < TABLE_SIZE; i++) {
        if (!strcmp(fileTable[i][0], extension)) {
            strcpy(type, fileTable[i][1]);
            return;
        }
    }
    strcpy(type, "application/octet-stream"); // Tipo padrão para arquivos desconhecidos

}

int readContent(char *path, Response *resp) {
    int fd;
    ssize_t i;

    if ((fd = open(path, O_RDONLY)) == -1) {
        resp->code = INTERNAL_ERROR;
        return -1; 
    }
    i = read(fd, resp->content, MAX_CONT);
    close(fd);
    return i;
}

void searchDir(char *path, Response *resp) {
    char resources[2][13] = {"/index.html", "/welcome.html"};
    char filename[MAX_NAME];
    int found = 0, read = 0, len, i;

    for (i = 0; i < 2; i++) {
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

void accessResource(char *dir, char *res, Response *resp, int depth, Login *login, int *protection) {
    struct stat resource_stats;;
    char path[MAX_NAME] = "", target[MAX_NAME] = "";
    char *next;
    int len, auth;

    // Verifica a existência de arquivo de proteção .htaccess
    checkProtection(dir, protection);

    // Se estiver tentando acessar algo fora do webspace
    if (depth < 0) {
        resp->code = FORBIDDEN;
        return;
    }

    // Monta o path para o recurso
    next = mystrtok(res, target, '/');
    strcat(path, dir);
    strcat(path, "/");
    strcat(path, target);

    // Acessa estatísticas do recurso
    if (stat(path, &resource_stats) == -1) {    // Se o recurso não for encontrado
        resp->code = NOT_FOUND;
        return;
    }

    switch (resource_stats.st_mode & S_IFMT) {
        // Se o recurso for um arquivo regular
        case S_IFREG:
            auth = authenticate(resp, login, protection);
            if (!auth) {
                resp->code = AUTH_REQUIRED;
                return;
            }
            if ((access(path, R_OK) != 0)) resp->code = FORBIDDEN;  // Se não tem permissão de leitura
            else {
                // Preenche Content-Type, Content-Length e Last-Modified
                getMediaType(resp->type, path);
                len = readContent(path, resp);
                resp->code = OK;
                resp->size = len;
                strcpy(resp->lmdate, asctime(localtime(&resource_stats.st_mtime)));
                resp->lmdate[strlen(resp->lmdate)-1] = 0;   // Tira quebra de linha do final
            }
            break;
        // Se o recurso for um diretório   
        case S_IFDIR:
            if (next == NULL) { // Se é o final do path
                checkProtection(path, protection);
                auth = authenticate(resp, login, protection);
                if (!auth) {
                    resp->code = AUTH_REQUIRED;
                    return;
                }
                if ((access(path, X_OK) != 0)) resp->code = FORBIDDEN;  // Se tem permissão de varredura
                else searchDir(path, resp);
            }
            else if (!strcmp(target, "..")) accessResource(path, next, resp, depth - 1, login, protection);  // Se for o diretório pai
            else if (!strcmp(target, ".")) accessResource(path, next, resp, depth, login, protection);  // Se for o próprio diretório
            else accessResource(path, next, resp, depth + 1, login, protection);  // Se for um diretório filho
            break;
    }
}

void flushResponse(int fd, Response *resp, int fields) {
    dprintf(fd, "HTTP/1.1 %d %s\r\n", resp->code, resp->result);  
    dprintf(fd, "Date: %s\r\n", resp->rdate);
    dprintf(fd, "Server: %s\r\n", resp->server);    
    dprintf(fd, "Connection: %s\r\n", resp->connection);
    if (fields & PRINT_TYPE_LENGTH) { 
        dprintf(fd, "Content-Type: %s\r\n", resp->type);
        dprintf(fd, "Content-Length: %d\r\n", resp->size);   
    }
    if (fields & PRINT_ALLOW) {
        dprintf(fd, "Allow: %s\r\n", resp->allow);
    }
    if (fields & PRINT_LM) {
        dprintf(fd, "Last-Modified: %s\r\n", resp->lmdate);  
    }
    if (fields & PRINT_AUTH) {
        dprintf(fd, "WWW-Authenticate: %s\r\n", resp->auth);  
    }
    if (fields & PRINT_CONTENT) {
        dprintf(fd, "\r\n");
        write(fd, resp->content, resp->size);
    }
    dprintf(fd, "\r\n");
}

void GET(char *res, Response *resp, int socket, Login *login) {
    int protection = -1;
    accessResource(webSpacePath, res, resp, 0, login, &protection);
    codeMsg(resp);
    if (resp->code == 200) {
        flushResponse(socket, resp, PRINT_TYPE_LENGTH | PRINT_CONTENT | PRINT_LM);
        flushResponse(logfile, resp, PRINT_TYPE_LENGTH | PRINT_LM);
    }
    else {
        httpError(socket, resp, "");
    }
}

void HEAD(char *res, Response *resp, int socket, Login *login) {
    int protection = -1;
    accessResource(webSpacePath, res, resp, 0, login, &protection);
    codeMsg(resp);
    if (resp->code == 200) {
        flushResponse(socket, resp, PRINT_TYPE_LENGTH | PRINT_LM);
        flushResponse(logfile, resp, PRINT_TYPE_LENGTH | PRINT_LM);
    }
    else {
        httpError(socket, resp, "");
    }
}

void OPTIONS(char *res, Response *resp, int socket, Login *login) {
    int protection = -1;
    accessResource(webSpacePath, res, resp, 0, login, &protection);
    codeMsg(resp);
    if (resp->code == 200) {
        flushResponse(socket, resp, PRINT_ALLOW);
        flushResponse(logfile, resp, PRINT_ALLOW);
    }
    else {
        httpError(socket, resp, "");
    }
}

void TRACE(char *res, Response *resp, int socket) {
    resp->code = OK;
    strcpy(resp->type, "message/http");
    flushResponse(socket, resp, PRINT_TYPE_LENGTH | PRINT_CONTENT);
    flushResponse(logfile, resp, PRINT_TYPE_LENGTH);
}

void extractLogin(listptr mainList, Login *login) {
    CommandNode *auth;
    char *encoded, *decoded, *temp1, *temp2;
    
    auth = searchCommand(mainList, "Authorization");
    if (auth != NULL) { // Se há um campo "Authorization" na requisição
        login->exists = 1;
        encoded = auth->paramList->parameter;
        char tok[strlen(encoded)];
        while (encoded != NULL) {
            encoded = mystrtok(encoded, tok, ' ');
        }
        decoded = b64_decode((const char*)tok);
        temp1 = malloc(strlen(decoded)*sizeof(char));
        temp2 = mystrtok(decoded, temp1, ':');
        strncpy(login->password, temp2, MAX_AUTH);
        strncpy(login->user, temp1, MAX_AUTH);
        login->password[MAX_AUTH] = 0;  // Trunca senha para 8 caracteres
        login->user[MAX_AUTH] = 0;      // Trunca usuário para 8 caracteres
        free(decoded);
        free(temp1);
        char *encrypted = crypt(login->password, DEFAULT_SALT);
        strcpy(login->password, encrypted);
    }
    else login->exists = 0;
}

int authenticate(Response *resp, Login *login, int *protection) {
    char htacc_cont[(2*MAX_NAME)+1], realm[MAX_NAME], user[MAX_AUTH+1];
    char *htpass_path, *line, *password;
    int len, htpass_fd, match = 0, authorized = 1;
    size_t linesize = MAX_AUTH+1+CRYPT_OUTPUT_SIZE+1;
    FILE *htpass_stream;

    if (*protection > 0) {  // Se existe um arquivo .htaccess protegendo o recurso alvo
        len = read(*protection, htacc_cont, (2*MAX_NAME)+1);
        htacc_cont[len] = 0;
        htpass_path = mystrtok(htacc_cont, realm, '\n');
        close(*protection);
        if (login->exists) {    // Se há credenciais fornecidas
            htpass_fd = open(htpass_path, O_RDONLY);
            htpass_stream = fdopen(htpass_fd, "r");
            line = malloc(linesize*sizeof(char));
            while ((!match) && len > 0) {
                len = getline(&line, &linesize, htpass_stream);
                if (line[len-1] == '\n') line[len-1] = 0;
                password = mystrtok(line, user, ':');
                if (!strcmp(user, login->user)) match = 1;
            }
            close(htpass_fd);
            if (match) match = (!strcmp(password, login->password)); // Se o usuário consta e as senhas coincidem
            free(line);
        }
        if (!match) {  // Pede por credenciais
            authorized = 0;
            strcpy(resp->auth, "Basic realm=");
            strcat(resp->auth, realm);
        }
    }
    return authorized;
}

void checkProtection(char *dir, int *current) {
    char htacc_path[MAX_NAME] = "";
    struct stat htaccess_stats;
    int htacc, aux;

    strcat(htacc_path, dir);
    strcat(htacc_path, "/.htaccess");
    if (stat(htacc_path, &htaccess_stats) != -1) { // Se achar um arquivo .htaccess
        htacc = open(htacc_path, O_RDONLY);
        aux = *current;
        *current = htacc;
        if (aux > 0) close(aux);
    }
}

int processRequest(listptr mainList, int socket) {
    Response resp;
    CommandNode *list;
    Login login;
    char *method, *resource;
    
    resp = createResponse();
    list = *mainList;
    method = list->command;
    resource = list->paramList->parameter;

    extractLogin(mainList, &login);

    if (strcmp(method, "GET") == 0) {
       GET(resource, &resp, socket, &login); 
    }
    else if (strcmp(method, "HEAD") == 0) {
        HEAD(resource, &resp, socket, &login);
    }
    else if (strcmp(method, "TRACE") == 0) {
        printOriginal(resp.content, mainList);
        TRACE(resource, &resp, socket);
    }
    else if (strcmp(method, "OPTIONS") == 0) {
        OPTIONS(resource, &resp, socket, &login);
    }

    return resp.code;
}
