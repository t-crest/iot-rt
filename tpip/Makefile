CC=gcc
# CC=clang

SERIAL?=/dev/ttyUSB0

help:
	# Make file for tpip
	#   pcap: tcpdump stuff
    #   wcet: wcet of one function and some png call graphs
	#   macrocheck: see how macros are tested
	#   slip: (ms: will you write what it does?)
	#   tpiphost: host listener (UDP on serial 2)
	#   tpippatmos: patmos will send UDP over serial 2
	#   clean: removing files



pcap:
	@echo 'tcpdump output:'
	#tcpdump -r tpip.pcap
	#tcpdump -nnvXSs 0 -c1 -i any

wcet:
	#patmos-clang -O2 -mserialize=tpip.pml tpip.c
#patmos-clang -O0 -mserialize=tpip.pml tpip.c
#the function to be analyzed in listed just before --report
#functions: checksum waitfornextperiod wait
#platin wcet -i tpip.pml -b a.out -e getipdstip  --report
#platin wcet -i tpip.pml -b a.out -e wait --report
	#platin wcet -i tpip.pml -b a.out -e waitfornextperiod
	#platin visualize -i tpip.pml -f waitfornextperiod
#platin wcet -i tpip.pml -b a.out -e currenttimemillis --report
#platin wcet -i tpip.pml -b a.out -e calculateipchecksum --report

#use this target to see how macros are expanded
macrocheck:
	$(CC) -E *.c

slip:
	$(CC) test_slip.c slip.c
	./a.out /dev/ttyUSB0

tpiphost:
    #start listening on host
	$(CC) tpip.c test_tpip.c
	./a.out

tpippatmos:
	#start on patmos (a second terminal)
	patmos-clang -O2 test_obb.c tpip.c -o test_obb.elf
	patserdow -v /dev/ttyUSB0 test_obb.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf