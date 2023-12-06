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
    char auth[MAX_PARAM];
} Response;

// Cria a struct resposta e preenche os parâmetros Date, Server e Connection.
Response createResponse();

// Cria uma página html de erro conforme o código de RESP, contendo a mensagem MESSAGE e imprime em SOCKET.
void httpError(int socket, Response *resp, const char *message);

// Armazena em BUF o conteúdo de um recurso em PATH.
int readContent(char *path, Response *resp);

// Verifica a existência de arquivo .htaccess.
int authenticate(char *dir, Response *resp);

// Stores the type of FILENAME in TYPE.
void getMediaType(char *type, char *filename);

// Percorre o diretório buscando os arquivos padrão (index.html e welcome.html).
void searchDir(char *path, Response *resp);

// Acessa as estatísticas do recurso RES no diretório DIR, preenche parâmetros e conteúdo da resposta.
void accessResource(char *dir, char *res, Response *resp, int depth);

// Guarda em RESULT a mensagem correspondente a CODE.
void codeMsg(Response *resp);

// Imprime os campos de RESP especificados em FIELDS no descritor FD.
void flushResponse(int fd, Response *resp, int fields);

// Monta a resposta referente a uma requisição do tipo GET.
void GET(char *path, Response *resp, int socket);

// Monta a resposta referente a uma requisição do tipo OPTIONS.
void OPTIONS(char *path, Response *resp, int socket);

// Monta a resposta referente a uma requisição do tipo TRACE.
void TRACE(char *path, Response *resp, int socket);

// Monta o PATH e chama o método HTTP adequado.
int processRequest(listptr mainList, int socket);
