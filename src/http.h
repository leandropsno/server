#define MAX_REQ 1024
#define MAX_CONT 4096
#define MAX_NAME 128
#define MAX_CMD 8
#define MAX_PARAM 64
#define NOT_FOUND 404
#define FORBIDDEN 403
#define OK 200
#define INT_ERROR 500

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
} Response;

// Cria a struct resposta e preenche os parâmetros Date, Server e Connection.
Response createResponse();

// Armazena em BUF o conteúdo de um recurso em PATH.
int readContent(char *path, char *buf);

// Percorre o diretório buscando os arquivos padrão (index.html e welcome.html).
void searchDir(char *path, Response *resp);

// Acessa as estatísticas do recurso em PATH, preenche parâmetros e conteúdo da resposta.
void accessResource(char *path, Response *resp);

// Guarda em MSG a mensagem correspondente a CODE.
void codeMsg(char *msg, int code);

// Monta a resposta referente a uma requisição do tipo GET.
void GET(char *path, Response *resp);

// Monta a resposta referente a uma requisição do tipo OPTIONS.
void OPTIONS(char *path, Response *resp);

// Monta a resposta referente a uma requisição do tipo TRACE.
void TRACE(char *path, Response *resp);

// Monta o PATH e chama o método HTTP adequado.
int processRequisition(char *host, char *resource);