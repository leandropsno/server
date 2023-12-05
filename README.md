# server

This is a simple HTTP Web Server written in C language. The main purpose of this project was to practice *Operating Systems* and *Computer Network* fundamentals.

The HTTP requests are parsed by a Flex-Bison analyzer ([http.l](src/http.l) and [http.y](src/http.y)).

## Version 11 - new features:
* Multi-threading

## How to Run
Run `./compile` and then `src/server <webspace> <N> <port>`, where:
* `webspace` is the path to your Web Space;
* `N` is the maximum number of parallel HTTP requests the server will be able to process;
* `port` is the number of an available port on your machine, from which you'll be receiving HTTP requests and sending HTTP responses.
