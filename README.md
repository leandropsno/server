# server

This is a simple HTTP Web Server written in C language. The main purpose of this project was to practice *Operating Systems* and *Computer Network* fundamentals.

The HTTP requests are parsed by a Flex-Bison analyzer ([http.l](src/http.l) and [http.y](src/http.y)).

## Version 13 - new features:
* Support to POST requests;
* Authentication methods using Base64 encoding and password encrypting;

## How to Run
Run `./compile.sh` and then `src/server <webspace> <N> <port> <logfile> <charset>`, where:
* `webspace` is the path to your Web Space;
* `N` is the maximum number of parallel HTTP requests the server will be able to process;
* `port` is the number of an available port on your machine, from which you'll be receiving HTTP requests and sending HTTP responses.
* `logfile` is the path to a log file, in which the HTTP requests and response headers will be logged.
* `charset` is the expected encoding for text files (utf-8, ISO-8859-1 etc).

To stop execution, just send an interruption signal (Ctrl+C).