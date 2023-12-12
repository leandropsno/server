/*
    All functions regarding Base64 decoding/encoding were developed by John's blog
    in https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/.
*/

#include <stdlib.h>

int b64_isvalidchar(char c);

size_t b64_decoded_size(const char *in);

/* Decodes the encoded string in IN. Returns a pointer to the decoded string. */
char *b64_decode(const char *in);

/* Splits the string in STR in tokens separated by the character DELIM.
The token is copied to TOK and a pointer to the beggining of the next token is returned, or NULL
if the end of the string was found.
The NULL-terminator character is added in the end of TOK. STR remains unaltered.
TOK must be previously allocated. The return pointer is STR plus the length of TOK plus 1 (because
of the delimiter).
*/
char *mystrtok(char *str, char *tok, char delim);
