CC = gcc

all: server client

server: server.c
	$(CC) server.c -o server

client: client.c
	$(CC) client.c -o client

clean:
	rm -f *.o server client

clean-fifo:
	sudo rm -f /tmp/task12fifo

.PHONY: clean