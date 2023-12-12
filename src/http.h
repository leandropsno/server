#include <crypt.h>

// Maximum size of strings
#define MAX_PARAM 64
#define MAX_CONT 4096
#define MAX_REQ 2048
#define MAX_NAME 256
#define MAX_CMD 8
#define MAX_AUTH 8

// HTTP response status codes
#define OK 200
#define AUTH_REQUIRED 401
#define FORBIDDEN 403
#define NOT_FOUND 404
#define BAD_REQUEST 400
#define INTERNAL_ERROR 500

// Response flushing field selection
#define PRINT_TYPE_LENGTH 1
#define PRINT_LM 2
#define PRINT_ALLOW 4
#define PRINT_AUTH 8
#define PRINT_CONTENT 16

// Others
#define TABLE_SIZE 31
#define DEFAULT_SALT "84"
#define CHUNK_SIZE 256

typedef struct Response {
    int code;
    int size;
    char result[MAX_PARAM];
    char rdate[MAX_PARAM];
    char server[MAX_PARAM];
    char connection[MAX_PARAM];
    char *content;
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

/* Creates anda returns a new Response. 
   The fields Data, Server, Connection and Allow are initialized with default values.
   The field Content-Length is initialized with 0.
   Content is dynamically allocated and MUST be freed after the use of the struct. */
Response createResponse();

/* Create an HTML page displaying the response code and MESSAGE, stores the page in the
   response content and flushes to SOCKET. */
void httpPage(int socket, Response *resp, const char *message);

/* Stores in *BUF the content of an open file with descriptor FD.
   *BUF must have been dynamically allocated, as the function realloc's it if necessary.
   Returns the amount of bytes read, not counting the NULL-terminator character which IS
   appended to *BUF. */ 
int readContent(int fd, char **buf);

/* Checks if an .htaccess file exists in DIR.
   If it does, opens it and stores the file descriptor in *CURRENT.
   If *CURRENT was previously set with another file descriptor, it is closed. */
void checkProtection(char *dir, int *current);

/* Stores the type of FILENAME in TYPE. */
void getMediaType(char *type, char *filename);

/* Searches for default files (index.html and welcome.html) in PATH.
   If any of them is found and has read permission, reads its content and
   changes RESP accordingly. */
void searchDir(char *path, Response *resp);

/* Acessess the stats of the resource RES in the directory DIR.
   If it is found and has read permission, reads its content and
   changes RESP accordingly.
   This function is recursive and only returns when the end of RES is reached. The parameter
   DEPTH indicates how many levels DIR is inside of the webspace root. */
void accessResource(char *dir, char *res, Response *resp, int depth, Login *login, int *protection);

/* Stores in resp->result the message that corresponds to resp->code. */ 
void codeMsg(Response *resp);

/* Writes an HTTP response message in the file descriptor FD.
   FIELDS specifies which fields of RESP are to be written, in the form of a bit-OR between
   the macros PRINT_TYPE_LENGTH, PRINT_LM, PRINT_ALLOW, PRINT_AUTH and PRINT_CONTENT. */ 
void flushResponse(int fd, Response *resp, int fields);

/* Authenticates using the credentials stored in LOGIN and the protection file open in PROTECTION.
   Returns 0 if authentication failed (incorrect or non-existent credentials) or 1 if
   successful (corrected credential or non-existent protection).
   Manipulation of RESP varies for each situation mentioned above.
*/
int authenticate(Response *resp, Login *login, int *protection);

/* Searches for the "Authorization" field in MAINLIST. If it is found, extracts the information
   and stores it in LOGIN. */
void extractLogin(listptr mainList, Login *login);

/* Builds the response to a GET request. */
void GET(char *path, Response *resp, int socket, Login *login);

/* Builds the response to a HEAD request. */
void HEAD(char *res, Response *resp, int socket, Login *login);

/* Builds the response to a OPTIONS request. */
void OPTIONS(char *path, Response *resp, int socket, Login *login);

/* Builds the response to a TRACE request. */
void TRACE(char *path, Response *resp, int socket);

/* Builds the response to a POST request. */
void POST(char *res, Response *resp, int socket, Login *login);

/* Extracts login and calls the HTTP method specified in MAINLIST. */
int processRequest(listptr mainList, int socket);
