# Makefile for tipsi

all: clean tipsi run

tipsi.o: tipsi.c tipsi.h
	gcc -std=c99 -c tipsi.c -o tipsi.o -I.

tipsi: tipsi.o
	gcc tipsi.o -o tipsi

run: tipsi
	./tipsi

clean:
	-rm -f tipsi.o tipsi.exe tipsi