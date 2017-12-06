CC=gcc
default: all
# CC=clang

#use this target to see how macros are expanded
#macrocheck:
#	$(CC) -E *.c

wcet:
	rm -f tpip.pml a.out
	patmos-clang -O2 -mserialize=tpip.pml tpip.c
	#patmos-clang -O0 -mserialize=tpip.pml tpip.c
	#the function to be analyzed in listed just before --report
	#functions: checksum waitfornextperiod wait
	platin wcet -i tpip.pml -b a.out -e mod --report

all:
	rm -f a.out #quiet if a.out is not there
	$(CC) *.c
	./a.out
	
