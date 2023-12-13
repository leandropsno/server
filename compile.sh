# Entre no diretório de arquivos fonte src
cd src

# Execute o bison com opção -d para gerar tabela de símbolos
bison -d http.y

# Execute o flex para gerar o lexer
flex http.l

# Compile o programa usando todos os arquivos utilitários e flags necessárias
gcc -o server server.c lex.yy.c http.c lists.c ast.c http.tab.c -lfl -ly -lpthread -lcrypt -g

# Volte para a raiz do servidor - o programa deve ser executado DAQUI da seguinte forma: src/server ../webspace N PORT log.txt charset
cd ..

# Para encerrar a execução do servidor, dê um sinal de interrupção (Ctrl+C).