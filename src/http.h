#define MAX_PARAM 64
#define MAX_CONT 4096

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

// Prints, both in response file and log file, the response common header fields (code, date, server, connection).
void flushCommonHeader(Response *resp);

// Prints, both in response file and log file, the response header fields related to content (last-modified, type, length).
void flushContentHeaders(Response *resp);

// Monta a resposta referente a uma requisição do tipo GET.
void GET(char *path, Response *resp);

// Monta a resposta referente a uma requisição do tipo OPTIONS.
void OPTIONS(char *path, Response *resp);

// Monta a resposta referente a uma requisição do tipo TRACE.
void TRACE(char *path, Response *resp);

// Monta o PATH e chama o método HTTP adequado.
int processRequisition(char *method, char *host, char *resource);