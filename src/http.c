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
#include <poll.h>
#include <crypt.h>
#include "lists.h"
#include "ast.h"
#include "http.h"

// Tabela de Media Types
const char *fileTable[TABLE_SIZE][2] = {
    {"htm", "text/html"}, {"html", "text/html"}, {"txt", "text/plain"}, {"css", "text/css"}, {"csv", "text/csv"}, {"js", "text/javascript"},
    {"avif", "image/avif"}, {"bmp", "image/bmp"}, {"gif", "image/gif"}, {"jpg", "image/jpeg"},  {"jpeg", "image/jpeg"}, {"png", "image/png"}, {"tif", "image/tiff"}, {"tiff", "image/tiff"}, {"webp", "image/webp"},
    {"zip", "application/zip"}, {"gzip", "application/gzip"}, {"json", "application/json"},  {"pdf", "application/pdf"}, {"rar", "application/x-rar-compressed"}, {"tar", "application/x-tar"}, {"rtf", "application/rtf"}, {"sh", "application/x-sh"}, {"xhtml", "application/xhtml+xml"}, {"xls", "application/vnd.ms-excel"}, {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"}, {"xml", "application/xml"},
    {"mp3", "audio/mpeg"}, {"wav", "audio/wav"}, {"mp4", "video/mp4"}, {"webm", "video/webm"}
};

extern int logfile;
extern char webSpacePath[MAX_NAME], charset[MAX_PARAM];

Response createResponse() {
    Response resp;
    struct timeval tv;

    gettimeofday(&tv, NULL);    // Obtem o horário do sistema
    strcpy(resp.rdate, asctime(localtime(&tv.tv_sec))); // Preenche parâmetro Date
    resp.rdate[strlen(resp.rdate)-1] = 0;   // Tira quebra de linha do final
    strcpy(resp.server, "Servidor HTTP versão 13 de Leandro Ponsano");  // Preenche parâmetro Server
    strcpy(resp.connection, "close"); // Preenche parâmetro Connection
    strcpy(resp.allow, "GET, HEAD, OPTIONS, TRACE"); // Preenche parâmetro Allow
    resp.size = 0;  // Inicializa parâmetro Content-Length com 0
    resp.content = (char *)malloc(sizeof(char) * CHUNK_SIZE);   // Aloca memória para o conteúdo da resposta

    return resp;
}

void httpPage(int socket, Response *resp, const char *message) {
    // Cria a página HTML e armazena no conteúdo da resposta
    sprintf(resp->content, "<!DOCTYPE html>\n<html>\n\t<head>\n\t\t<title>%d</title>\n\t</head>\n\t<body>\n\t\t<h1>%d %s</h1>\n\t\t<p>%s</p>\n\t</body>\n</html>", resp->code, resp->code, resp->result, message);
    resp->size = strlen(resp->content); // Preenche parâmetro Content-Length
    getMediaType(resp->type, ".html");    // Preenche parâmetro Content-Type
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
        case BAD_REQUEST:
            strcpy(resp->result, "Bad Request");
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
            if (i <= 5) {
                strcat(type, "; charset=");
                strcat(type, charset);
            }
            return;
        }
    }
    strcpy(type, "application/octet-stream"); // Tipo padrão para arquivos desconhecidos

}

int readContent(int fd, char **buf) {
    ssize_t i = CHUNK_SIZE, total = 0, size = CHUNK_SIZE;

    // Lê um chunk de conteúdo
    while (i >= CHUNK_SIZE) {
        i = read(fd, *buf + total, CHUNK_SIZE);
        total += i;
        if (i == CHUNK_SIZE) {  // Se um chunk inteiro foi lido
            size += CHUNK_SIZE;
            *buf = (char *)realloc(*buf, size); // Realoca o buffer de um chunk
            if (*buf == NULL) { // Se houve falha na realocação
                return -1;
            }
        }
    }
    char *end = *buf;
    end += total;
    *end = 0;
    return total;
}

void searchDir(char *path, Response *resp) {
    char resources[2][13] = {"/index.html", "/welcome.html"};
    char filename[MAX_NAME];
    int found = 0, read = 0, len, i, fd;

    for (i = 0; i < 2; i++) {
        // Monta path para o recurso padrão
        strcpy(filename, path);
        strcat(filename, resources[i]);

        // Acessa estatísticas do recurso
        struct stat file_stats;
        if (stat(filename, &file_stats) == 0) {
            found = 1;
            if (access(filename, R_OK) == 0) { // Se encontrou um arquivo com permissão de leitura
                if ((fd = open(filename, O_RDONLY)) == -1) {    // Se houve falha na abertura do arquivo
                    resp->code = INTERNAL_ERROR;
                    return; 
                }
                len = readContent(fd, &resp->content);
                close(fd);
                resp->code = OK;  // Preenche parâmetro Status Code
                read = 1;
                resp->size = len;   // Preenche parâmetro Content-Length
                getMediaType(resp->type, ".html");    // Preenche parâmetro Content-Type
                strcpy(resp->lmdate, asctime(localtime(&file_stats.st_mtime))); // Preenche parâmetro Last-Modified
                resp->lmdate[strlen(resp->lmdate)-1] = 0;   // Tira quebra de linha do final
                break;
            }
        }
    }

    if (!found) { // Se nenhum dos dois foi encontrado
        resp->code = NOT_FOUND;
    }
    else if (!read) {  // Se algum foi encontrado mas nenhum tinha permissão de leitura
        resp->code = FORBIDDEN;
    }
}

void accessResource(char *dir, char *res, Response *resp, int depth, Login *login, int *protection) {
    struct stat resource_stats;;
    char path[MAX_NAME] = "", target[MAX_NAME] = "";
    char *next;
    int len, auth, fd;

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
            auth = authenticate(resp, login, protection);   // Realiza a autenticação
            if (!auth) {    // Se a autenticação falhou
                resp->code = AUTH_REQUIRED;
                return;
            }
            if ((access(path, R_OK) != 0)) resp->code = FORBIDDEN;  // Se não tem permissão de leitura
            else {
                // Preenche Content-Type, Content-Length e Last-Modified
                getMediaType(resp->type, path);
                if ((fd = open(path, O_RDONLY)) == -1) {
                    resp->code = INTERNAL_ERROR;
                    return; 
                }
                len = readContent(fd, &resp->content);
                close(fd);
                resp->code = OK;
                resp->size = len;
                strcpy(resp->lmdate, asctime(localtime(&resource_stats.st_mtime)));
                resp->lmdate[strlen(resp->lmdate)-1] = 0;   // Tira quebra de linha do final
            }
            break;
        // Se o recurso for um diretório   
        case S_IFDIR:
            if ((access(path, X_OK) != 0)) resp->code = FORBIDDEN;  // Se tem permissão de varredura
            else if (next == NULL) { // Se é o final do path
                checkProtection(path, protection);
                auth = authenticate(resp, login, protection); // Realiza a autenticação
                if (!auth) { // Se a autenticação falhou
                    resp->code = AUTH_REQUIRED;
                    return;
                }
                searchDir(path, resp);
            }
            else if (!strcmp(target, "..")) accessResource(path, next, resp, depth - 1, login, protection);  // Se for o diretório pai
            else if (!strcmp(target, ".")) accessResource(path, next, resp, depth, login, protection);  // Se for o próprio diretório
            else accessResource(path, next, resp, depth + 1, login, protection);  // Se for um diretório filho
            break;
    }
}

void flushResponse(int fd, Response *resp, int fields) {
    // Imprime os campos de cabeçalho padrão
    dprintf(fd, "HTTP/1.1 %d %s\r\n", resp->code, resp->result);  
    dprintf(fd, "Date: %s\r\n", resp->rdate);
    dprintf(fd, "Server: %s\r\n", resp->server);    
    dprintf(fd, "Connection: %s\r\n", resp->connection);

    // Imprime os campos de cabeçalho especificados em FIELDS
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
        httpPage(socket, resp, "");
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
        httpPage(socket, resp, "");
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
        httpPage(socket, resp, "");
    }
}

void TRACE(char *res, Response *resp, int socket) {
    resp->code = OK;
    strcpy(resp->type, "message/http");
    flushResponse(socket, resp, PRINT_TYPE_LENGTH | PRINT_CONTENT);
    flushResponse(logfile, resp, PRINT_TYPE_LENGTH);
}

void POST(char *res, Response *resp, int socket, Login *login) {
    char path[MAX_NAME + 1] = "", aux[MAX_NAME + 1] = "", salt[3] = "";
    char userName[MAX_AUTH+1], oldPasswd[MAX_AUTH+1], newPasswd[MAX_AUTH+1], confPasswd[MAX_AUTH+1];
    char *next, *load = (char *)malloc(CHUNK_SIZE), *encrypted;
    int len = 0, fd, match = 0, n;
    off_t offset = 0;
    struct pollfd fdset;
    long unsigned int timeout = 1000;

    // Adiciona o descritor do socket no set de descritores
    fdset.events = POLLIN;
    fdset.fd = socket;
    // Aguarda o socket estar pronto para leitura
    n = poll(&fdset, 1, timeout);
    if ((n > 0) && (fdset.revents == POLLIN)) { 
        len = readContent(socket, &load); // Lê o conteúdo enviado pelo POST
    }
    if (len <= 0 || n == 0) {
        resp->code = BAD_REQUEST;
        codeMsg(resp);
        httpPage(socket, resp, "Nenhuma informação recebida.");
        return;
    }
    
    // Obtém os campos separados
    next = load + 5;    // Ignora "user="
    next = mystrtok(next, userName, '&');
    next = next + 7;    // Ignora "antiga="
    next = mystrtok(next, oldPasswd, '&');
    next = next + 5;    // Ignora "nova="
    next = mystrtok(next, newPasswd, '&');
    next = next + 5;    // Ignora "conf="
    next = mystrtok(next, confPasswd, '&');

    // Confere se a senha e a confirmação coincidem
    if (strcmp(newPasswd, confPasswd)) {
        resp->code = BAD_REQUEST;
        codeMsg(resp);
        httpPage(socket, resp, "Nova senha não coincide com a confirmação.");
        return;
    }
    
    // Monta o path para o .htaccess do diretório em questão
    next = res;
    strcat(path, webSpacePath);
    while (strcmp(next, "nova_senha.html")) { // Enquanto next for diferente de "nova_senha.html"
        next = mystrtok(next, aux, '/');
        strcat(path, aux);
        strcat(path, "/");
    }
    strcat(path, ".htaccess");

    // Abre e lê o conteudo do .htaccess
    if ((fd = open(path, O_RDONLY)) == -1) {
        resp->code = INTERNAL_ERROR;
        return; 
    }
    len = readContent(fd, &load);
    close(fd);
    next = mystrtok(load, path, '\n'); // next aponta para o path do .htpassword

    // Abre e lê o conteudo do .htpassword
    if ((fd = open(next, O_RDWR)) == -1) {
        resp->code = INTERNAL_ERROR;
        return; 
    }
    len = readContent(fd, &load);

    // Percorre o arquivo de senhas
    next = load;
    while (next != NULL) {  // Procura pelo nome de usuário no arquivo de senhas
        next = mystrtok(next, aux, ':');
        offset += strlen(aux) + 1;
        if (!strcmp(aux, userName)) {   
            match = 1;
            break;
        }
        next = mystrtok(next, aux, '\n');
        offset += strlen(aux) + 1;
    }
    // Se encontrou o usuário no arquivo de senhas
    if (match) {    
        next = mystrtok(next, aux, '\n'); // Obtém a senha registrada (criptografada)
        strncpy(salt, aux, 2);
        encrypted = crypt(oldPasswd, salt);
        match = (!strcmp(aux, encrypted)); 
    }
    if (match) {     // Se a senha criptografada coincide com a senha no arquivo de senhas
        encrypted = crypt(newPasswd, salt); // Crpitografa a nova senha
        offset = lseek(fd, offset, SEEK_SET); // Seta o offset para a posição da nova senha
        len = write(fd, encrypted, strlen(encrypted));  // Escreve a nova senha
        resp->code = OK;
        codeMsg(resp);
        httpPage(socket, resp, "Senha atualizada com sucesso.");
    }
    else {  // Caso o usuário não seja encontrado OU senhas não coincidam
        resp->code = BAD_REQUEST;
        codeMsg(resp);
        httpPage(socket, resp, "Usuário e/ou senha não conferem.");
    }
    free(load);
    close(fd);
}

void extractLogin(listptr mainList, Login *login) {
    CommandNode *auth;
    char *encoded, *decoded, *temp1, *temp2;
    
    auth = searchCommand(mainList, "Authorization");
    if (auth != NULL) { // Se há um campo "Authorization" na requisição
        login->exists = 1;
        encoded = auth->paramList->parameter;
        char tok[strlen(encoded)];
        while (encoded != NULL) { // Obtém o par usuário:senha codificada
            encoded = mystrtok(encoded, tok, ' ');
        }
        decoded = b64_decode((const char*)tok); // Decodifica o par usuário:senha
        temp1 = (char *)malloc(strlen(decoded)*sizeof(char));   
        temp2 = mystrtok(decoded, temp1, ':');  // Obtém a senha
        strncpy(login->password, temp2, MAX_AUTH);
        strncpy(login->user, temp1, MAX_AUTH);
        login->password[MAX_AUTH] = 0;  // Trunca senha para 8 caracteres
        login->user[MAX_AUTH] = 0;      // Trunca usuário para 8 caracteres
        free(decoded);
        free(temp1);
        char *encrypted = crypt(login->password, DEFAULT_SALT); // Criptografa a senha
        strcpy(login->password, encrypted); // Armazena a sneha criptografada no login
    }
    else login->exists = 0;
}

int authenticate(Response *resp, Login *login, int *protection) {
    char realm[MAX_NAME], user[MAX_AUTH+1];
    char *htpass_path, *line, *password, *htacc_cont = (char *)malloc((2*MAX_NAME)+1);
    int len, htpass_fd, match = 0, authorized = 1;
    size_t linesize = MAX_AUTH+1+CRYPT_OUTPUT_SIZE+1;
    FILE *htpass_stream;

    if (*protection > 0) {  // Se existe um arquivo .htaccess protegendo o recurso alvo
        len = readContent(*protection, &htacc_cont); // Lê o conteúdo do .htaccess
        htpass_path = mystrtok(htacc_cont, realm, '\n'); // Obtém acaminho para o .htpassword
        close(*protection);
        if (login->exists) {    // Se há credenciais fornecidas
            htpass_fd = open(htpass_path, O_RDONLY); // Abre .htpassword
            htpass_stream = fdopen(htpass_fd, "r"); // Gera um FILE* a partir do descritor
            line = (char *)malloc(linesize*sizeof(char));
            while ((!match) && len > 0) { // Procura pelo usuário no arquivo de senhas
                len = getline(&line, &linesize, htpass_stream); // Lê uma linha do .htpassword
                if (line[len-1] == '\n') line[len-1] = 0;   // Retira quebra de linha 
                password = mystrtok(line, user, ':');   // Separa usuário e senha
                if (!strcmp(user, login->user)) match = 1; 
            }
            close(htpass_fd);
            if (match) match = (!strcmp(password, login->password)); // Se o usuário consta e as senhas coincidem
            free(line);
        }
        if (!match) {  // Se não houve match, pede por credenciais
            authorized = 0;
            strcpy(resp->auth, "Basic realm=");
            strcat(resp->auth, realm);
        }
    }
    free(htacc_cont);
    return authorized;
}

void checkProtection(char *dir, int *current) {
    char htacc_path[MAX_NAME] = "";
    struct stat htaccess_stats;
    int htacc, aux;

    // Monta caminho para o .htaccess
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
    
    // Cria uma resposta e obtém os parâmetros da requisição
    resp = createResponse();
    list = *mainList;
    method = list->command;
    resource = list->paramList->parameter;
    
    // Procura por credenciais fornecidas e, se houver, as armazena em LOGIN
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
    else if (strcmp(method, "POST") == 0) {
        POST(resource, &resp, socket, &login);
    }

    free(resp.content);

    return resp.code;
}
