cd src
bison -d http.y
flex http.l
gcc -o server server.c lex.yy.c http.c lists.c http.tab.c -lfl -ly -g
clear
cd ..
