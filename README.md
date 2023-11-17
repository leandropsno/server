# server

This is a simple HTTP Web Server written in C language. The main purpose of this project was to practice *Operating Systems* and *Computer Network* fundamentals.

The HTTP requests are parsed by a Flex-Bison analyzer ([http.l](src/http.l) and [http.y](src/http.y)).

## How to Run
Run `./compile` and then `src/server <ws> <req> <resp> <log>`, where:
* `ws` is the full path to your Web Space;
* `req` is the path to a file containing an HTTP Request;
* `resp` is the path to a file in which the HTTP Response will be written;
* `log` is the path to a Log File in which all Requests and Responde Headers will be written.
