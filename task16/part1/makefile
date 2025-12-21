CC = gcc

all: unix_tcp_server unix_tcp_client unix_udp_server unix_udp_client \
inet_tcp_server inet_tcp_client inet_udp_server inet_udp_client

unix_tcp_server: unix_tcp_server.c
	$(CC) unix_tcp_server.c -o unix_tcp_server

unix_tcp_client: unix_tcp_client.c
	$(CC) unix_tcp_client.c -o unix_tcp_client

unix_udp_server: unix_udp_server.c
	$(CC) unix_udp_server.c -o unix_udp_server

unix_udp_client: unix_udp_client.c
	$(CC) unix_udp_client.c -o unix_udp_client

inet_tcp_server: inet_tcp_server.c
	$(CC) inet_tcp_server.c -o inet_tcp_server

inet_tcp_client: inet_tcp_client.c
	$(CC) inet_tcp_client.c -o inet_tcp_client

inet_udp_server: inet_udp_server.c
	$(CC) inet_udp_server.c -o inet_udp_server

inet_udp_client: inet_udp_client.c
	$(CC) inet_udp_client.c -o inet_udp_client

clean:
	rm -f *.o unix_tcp_server unix_tcp_client

.PHONY: clean