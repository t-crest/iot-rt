CC=gcc
default: all
# CC=clang

#use this target to see how macros are expanded
#macrocheck:
#	$(CC) -E *.c

wcet:
	@echo ' '
	@echo '*********************************************************'
	@echo '*********************************************************'
	@echo '**********************NEW WCET***************************'
	@echo '*********************************************************'
	@echo '*********************************************************'
	@echo ' '
	rm -f tpip.pml a.out
	patmos-clang -O2 -mserialize=tpip.pml tpip.c
	#patmos-clang -O0 -mserialize=tpip.pml tpip.c
	#the function to be analyzed in listed just before --report
	#functions: checksum waitfornextperiod wait
	#platin wcet -i tpip.pml -b a.out -e getipdstip  --report
	#platin wcet -i tpip.pml -b a.out -e wait --report
	platin wcet -i tpip.pml -b a.out -e waitfornextperiod
	platin visualize -i tpip.pml -f waitfornextperiod
	#platin wcet -i tpip.pml -b a.out -e currenttimemillis --report
	#platin wcet -i tpip.pml -b a.out -e calculateipchecksum --report

all:
	@echo ' '
	@echo '*********************************************************'
	@echo '*********************************************************'
	@echo '**********************NEW BUILD**************************'
	@echo '*********************************************************'
	@echo '*********************************************************'
	@echo ' '
	rm -f a.out #quiet if a.out is not there
	rm -f tpip.pcap
	$(CC) *.c
	@echo 'tpip.c output:'
	./a.out
	@echo ' '
	@echo 'tcpdump output:'
	#it fails but is is ok
	tcpdump -r tpip.pcap
	#note: sudo tcpdump -nnvXSs 0 -c1 -i any