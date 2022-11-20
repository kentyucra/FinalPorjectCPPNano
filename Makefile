all:
	g++ -g -std=c++14 main.cpp order.h socket.h ncursesdisplay.h -pthread -lcrypto -lssl -lncurses -o finalproject.o
