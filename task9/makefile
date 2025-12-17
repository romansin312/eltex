CC = gcc
CFLAGS = -fPIC
LDFLAGS = -ldl

all: calc add.so sub.so mul.so div.so

calc: main.o plugin_loader.o
	$(CC) main.o plugin_loader.o -o calc $(LDFLAGS)

main.o: main.c plugin_loader.h
	$(CC) -c main.c

plugin_loader.o: plugin_loader.c plugin_loader.h
	$(CC) -c plugin_loader.c

add.so: add.o
	$(CC) -shared add.o -o add.so

add.o: add.c
	$(CC) $(CFLAGS) -c add.c -o add.o

sub.so: sub.o
	$(CC) -shared sub.o -o sub.so

sub.o: sub.c
	$(CC) $(CFLAGS) -c sub.c -o sub.o

mul.so: mul.o
	$(CC) -shared mul.o -o mul.so

mul.o: mul.c
	$(CC) $(CFLAGS) -c mul.c -o mul.o

div.so: div.o
	$(CC) -shared div.o -o div.so

div.o: div.c
	$(CC) $(CFLAGS) -c div.c -o div.o

clean:
	rm -f *.o *.so calc

.PHONY: all clean run