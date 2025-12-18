CC = gcc

all: server client

server: server.c
	$(CC) server.c -o server

client: client.c
	$(CC) client.c -o client

clean:
	rm -f *.o server client

clean-mq:
	sudo rm -f /dev/mqueue/server_mq
	sudo rm -f /dev/mqueue/client_mq

.PHONY: clean