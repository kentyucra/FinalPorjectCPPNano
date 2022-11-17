all:
	g++ -g main.cpp socket.h -pthread -lcrypto -lssl -lncurses -o finalproject.o