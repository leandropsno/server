#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "http.h"
#include "lists.h"

CommandNode* mainList = NULL;
int logfile;
char webSpacePath[50];

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Uso: ./servidor <WebSpace> IP_Address Port_Number\n");
        exit(1);
    }

    int socket = connect2Server(argv[2], argv[3]);
    logfile = open("io/log.txt", O_CREAT | O_APPEND, 00700); 
    strcpy(webSpacePath, argv[1]);

    char requestMessage[MAX_CONT], responseMessage[MAX_CONT];

    int i, j, k, wr = 0;
    do {
        i = read(socket, requestMessage, 1024);
        if (i < 0) {
            // FIX ME
        }
        yy_scan_string(requestMessage);
        write(0, msg_volta, i);
        if (!wr) {
            if ((j = search_content(msg_volta)) >= 0) {
                wr = 1;
                k = write(registro, &msg_volta[j], i-j);
            }  
        }
        else k = write(registro, msg_volta, i);
    } while (i >= 1024);
    yyparse();
    
    close(socket);
    fclose(yyin);
    fclose(respfile);
    fclose(logfile);
    return 0;
}