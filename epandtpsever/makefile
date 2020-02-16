server.run : server.o thread_pool.o wrap.o http.o
	gcc server.o thread_pool.o wrap.o http.o -o server.run -pthread -Wall -g

server.o : epoll_server.c
	gcc epoll_server.c -c -o server.o -g

wrap.o : wrap.c
	gcc wrap.c -c

thread_pool.o:threadpool.c
	gcc threadpool.c -c -o thread_pool.o

http.o : http.c
	gcc http.c -c

clean :
	rm -f *.run *.o
