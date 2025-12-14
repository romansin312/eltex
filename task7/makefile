CC = gcc

calc: main.o libcalc.a
	$(CC) main.o -L. -lcalc -o calc

libcalc.a: add.o sub.o mul.o div.o
	ar cr libcalc.a add.o sub.o mul.o div.o

main.o: main.c calc.h
	$(CC) -c main.c

add.o: add.c add.h
	$(CC) -c add.c

sub.o: sub.c sub.h
	$(CC) -c sub.c

mul.o: mul.c mul.h
	$(CC) -c mul.c

div.o: div.c div.h
	$(CC) -c div.c

clean:
	rm -f *.o libcalc.a calc