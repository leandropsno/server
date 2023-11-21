# server

This is a simple HTTP Web Server written in C language. The main purpose of this project was to practice *Operating Systems* and *Computer Network* fundamentals.

The HTTP requests are parsed by a Flex-Bison analyzer ([http.l](src/http.l) and [http.y](src/http.y)).

## How to Run
Run `./compile` and then `src/server <ws> <port>`, where:
* `ws` is the full path to your Web Space;
* `port` is the number of an available port on your machine, from which you'll be sending HTTP requests and receiving HTTP responses.
