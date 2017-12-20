CC=gcc
default: all
# CC=clang

#use this target to see how macros are expanded
#macrocheck:
#	$(CC) -E *.c

all:
	@echo ' '
	@echo '********************************************************************************'
	@echo 'NEW NOCSIM BUILD****************************************************************'
	@echo '********************************************************************************'
	@echo ' '
	rm -f a.out #quiet if a.out is not there
	$(CC) -pthread *.c
	./a.out