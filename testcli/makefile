client.run:client.o wrap.o
	gcc client.o wrap.o -o client.run  -pthread -Wall 

client.o:client.c
	gcc -c client.c -g
wrap.o:wrap.c
	gcc -c wrap.c

clean:
	rm -f *.o *.run
