typedef struct ParamNode {
    char parameter[300];
    struct ParamNode* next;
} ParamNode;

typedef struct CommandNode {
    char command[40];
    ParamNode* paramList;
    struct CommandNode* next;
} CommandNode;

typedef struct CommandNode** listptr;

// Cria um nó de parâmetro e o devolve.
ParamNode* createParamNode(char* parameter);

// Cria um nó de comando e o devolve.
CommandNode* createCommandNode(char* command);

// Adiciona um nó de parâmetro no final da lista de parâmetros do último nó de comandos de uma listas de comando.
void addParam(listptr list, char* parameter);

// Adiciona um nó de comando no final de uma lista de comandos.
void addCommand(listptr list, char* command);

// Recria e armazena em buf a sequência de caracteres que originou a lista.
void printOriginal(char *buf, listptr list);

// Imprime uma lista de comandos.
void printCommandList(listptr list);

// Libera o espaço alocado por uma lista de parametros.
void freeParamList(ParamNode* ini);

// Libera o espaço alocado por uma lista de comandos e seus parâmetros.
void freeCommandList(CommandNode *ini);

// Limpa uma lista.
void cleanupList(listptr list);