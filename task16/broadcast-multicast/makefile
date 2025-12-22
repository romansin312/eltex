CC = gcc

all: broadcast-server broadcast-client multicast-server multicast-client

broadcast-server: broadcast-server.c
	$(CC) broadcast-server.c -o broadcast-server

broadcast-client: broadcast-client.c
	$(CC) broadcast-client.c -o broadcast-client

multicast-server: multicast-server.c
	$(CC) multicast-server.c -o multicast-server

multicast-client: multicast-client.c
	$(CC) multicast-client.c -o multicast-client

clean:
	rm -f *.o unix_tcp_server unix_tcp_client

.PHONY: clean