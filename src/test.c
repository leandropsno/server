#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crypt.h>
#include "ast.h"
#include "lists.h"
#include "http.h"

char webSpacePath[50] = "../webspace";

int main(int argc, char **argv)
{
	CommandNode *list = NULL;
	addCommand(&list, "GET");
	addParam(&list, "/dir4/dir41/index.html");
	addParam(&list, "HTTP/1.1");
	addCommand(&list, "Authorization");
	addParam(&list, "Basic YWRtaW46YWRtaW4=");
	processRequest(&list, 0);
	return 0;
}
