all:
	g++ -g main.cpp order.h socket.h ncursesdisplay.h -pthread -lcrypto -lssl -lncurses -o finalproject.o