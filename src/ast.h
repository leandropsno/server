#include <stdlib.h>

int b64_isvalidchar(char c);

size_t b64_decoded_size(const char *in);

char *b64_decode(const char *in);

// Just like strtok, but sets token in TOK and returns a pointer to the beggining of the next token (or NULL if there will be no next token).
char *mystrtok(char *str, char *tok, char delim);
