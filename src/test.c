#include "lists.h"
#include "http.h"
#include "ast.h"

int logfile = 0;
char webSpacePath[50] = "../webspace";

void main() {

    char resource[] = "";
    Response resp = createResponse();
    GET(resource, &resp, 0); 

}