CC = gcc
CFLAGS = -pthread

all: server_thread_per_client server_thread_pool server_producer_consumer server_multiprotocol client_tcp client_udp

server_thread_per_client: server_thread_per_client.c
	$(CC) $(CFLAGS) server_thread_per_client.c -o server_thread_per_client

server_thread_pool: server_thread_pool.c
	$(CC) $(CFLAGS) server_thread_pool.c -o server_thread_pool

server_producer_consumer: server_producer_consumer.c
	$(CC) $(CFLAGS) server_producer_consumer.c -o server_producer_consumer

server_multiprotocol: server_multiprotocol.c
	$(CC) server_multiprotocol.c -o server_multiprotocol

client_tcp: client_tcp.c
	$(CC) client_tcp.c -o client_tcp

client_udp: client_udp.c
	$(CC) client_udp.c -o client_udp

clean:
	rm -f server_thread_per_client server_thread_pool server_producer_consumer server_multiprotocol client_tcp client_udp

.PHONY: clean all