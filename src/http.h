#include <crypt.h>
#define MAX_PARAM 64
#define MAX_CONT 4096
#define MAX_REQ 2048
#define MAX_NAME 256
#define MAX_CMD 8
#define MAX_AUTH 8
#define NOT_FOUND 404
#define FORBIDDEN 403
#define AUTH_REQUIRED 401
#define OK 200
#define INTERNAL_ERROR 500
#define PRINT_TYPE_LENGTH 1
#define PRINT_LM 2
#define PRINT_ALLOW 4
#define PRINT_AUTH 8
#define PRINT_CONTENT 16
#define TABLE_SIZE 31
#define DEFAULT_SALT "84"

typedef struct Response {
    int code;
    int size;
    char result[MAX_PARAM];
    char rdate[MAX_PARAM];
    char server[MAX_PARAM];
    char connection[MAX_PARAM];
    char content[MAX_CONT];
    char lmdate[MAX_PARAM];
    char type[MAX_PARAM];
    char allow[MAX_PARAM];
    char auth[MAX_PARAM];
} Response;

typedef struct Login {
    int exists;
    char user[MAX_AUTH+1];
    char password[CRYPT_OUTPUT_SIZE];
} Login;

// Cria a struct resposta e preenche os parâmetros Date, Server e Connection.
Response createResponse();

// Cria uma página html de erro conforme o código de RESP, contendo a mensagem MESSAGE e imprime em SOCKET.
void httpError(int socket, Response *resp, const char *message);

// Armazena em BUF o conteúdo de um recurso em PATH.
int readContent(char *path, Response *resp);

// Verifica a existência de arquivo .htaccess.
void checkProtection(char *dir, int *current);

// Stores the type of FILENAME in TYPE.
void getMediaType(char *type, char *filename);

// Percorre o diretório buscando os arquivos padrão (index.html e welcome.html).
void searchDir(char *path, Response *resp);

// Acessa as estatísticas do recurso RES no diretório DIR, preenche parâmetros e conteúdo da resposta.
void accessResource(char *dir, char *res, Response *resp, int depth, Login *login, int *protection);

// Guarda em RESULT a mensagem correspondente a CODE.
void codeMsg(Response *resp);

// Imprime os campos de RESP especificados em FIELDS no descritor FD.
void flushResponse(int fd, Response *resp, int fields);

/* Se houver descritor de um arquivo de proteção aberto em PROTECTION, realiza a autenticação usando as credenciais em LOGIN.
Retorna 0 se a autenticação falhou (credencial incorreta ou inexistente) e 1 se foi bem sucedida (credencial correta ou proteção inexistente).
A manipulação de RESP varia para cada uma das situações mencionadas.
*/
int authenticate(Response *resp, Login *login, int *protection);

// Procura por um campo "Authorization" em MAINLIST, e se houver, extrai as informações e armazena em LOGIN.
void extractLogin(listptr mainList, Login *login);

// Monta a resposta referente a uma requisição do tipo GET.
void GET(char *path, Response *resp, int socket, Login *login);

// Monta a resposta referente a uma requisição do tipo HEAD.
void HEAD(char *res, Response *resp, int socket, Login *login);

// Monta a resposta referente a uma requisição do tipo OPTIONS.
void OPTIONS(char *path, Response *resp, int socket, Login *login);

// Monta a resposta referente a uma requisição do tipo TRACE.
void TRACE(char *path, Response *resp, int socket);

// Monta o PATH e chama o método HTTP adequado.
int processRequest(listptr mainList, int socket);
