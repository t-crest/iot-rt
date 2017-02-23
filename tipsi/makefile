default: tipsi

tipsi.o: tipsi.c
	gcc -std=c99 -c tipsi.c -o tipsi.o

tipsi: tipsi.o
	gcc tipsi.o -o tipsi

clean:
	-rm -f tipsi.o
	-rm -f tipsi
