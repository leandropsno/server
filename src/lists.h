typedef struct ParamNode {
    char parameter[300];
    struct ParamNode* next;
} ParamNode;

typedef struct CommandNode {
    char command[40];
    ParamNode* paramList;
    struct CommandNode* next;
} CommandNode;

// Cria um nó de parâmetro e o devolve.
ParamNode* createParamNode(char* parameter);

// Cria um nó de comando e o devolve.
CommandNode* createCommandNode(char* command);

// Adiciona um nó de parâmetro no final da lista de parâmetros do último nó de comandos de uma listas de comando.
void addParam(CommandNode** cmd, char* parameter);

// Adiciona um nó de comando no final de uma lista de comandos.
void addCommand(CommandNode** ini, char* command);

// Imprime uma lista de comandos.
void printCommandList(CommandNode* ini);

// Libera o espaço alocado por uma lista de parametros.
void freeParamList(ParamNode* ini);

// Libera o espaço alocado por uma lista de comandos e seus parâmetros.
void freeCommandList(CommandNode* ini);

// Limpa uma lista de comandos (imprime os valores e libera os ponteiros).
void cleanupList(CommandNode *list);