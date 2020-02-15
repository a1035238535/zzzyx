server : server.o wrap.o
	gcc server.o wrap.o -o server

server.o : epoll_server.cpp
	gcc epoll_server.cpp -c -o server.o

wrap.o : wrap.cpp
	gcc wrap.cpp -c

clean :
	rm -f server *.o
