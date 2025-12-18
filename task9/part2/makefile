CC = gcc

filemanager: main.o
	$(CC) main.o -o filemanager -lncurses

main.o: main.c
	$(CC) -c main.c

clean:
	rm -f *.o filemanager

.PHONY: clean