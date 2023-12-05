#include <stdlib.h>

int b64_isvalidchar(char c);

size_t b64_decoded_size(const char *in);

int b64_decode(const char *in, unsigned char *out, size_t outlen);

// Just like strtok, but sets NEXT to the beggining of the next token (or NULL if there will be no next token).
char *mystrtok(char *str, char *delim, char *next);