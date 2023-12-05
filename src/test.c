#include "http.h"

void main() {

    char resource[] = "dir1/dir2/../index.html";
    Response resp = createResponse();
    GET(resource, &resp, 0); 

}