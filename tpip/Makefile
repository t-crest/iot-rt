CC=gcc
# CC=clang
PATMOSHOME=~/t-crest/patmos
SERIAL?=/dev/ttyUSB0
LIBETH=$(PATMOSHOME)/c/ethlib
TCP_ROLE=SERVER
include $(PATMOSHOME)/c/Makefile
# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
	-I$(PATMOSHOME)/c -I $(PATMOSHOME)/c/libelf/ \
	-I $(LIBETH) \
	-mpatmos-disable-vliw \
	$(DEFINES)

#TODO: make a use-case build system that takes an argument (use-case number)
#      and runs the use-case. This should be done for the two targets tpiphost and tpippatmos

help:
	# Make file for tpip
	#   pcap: tcpdump stuff
  	#   wcet: wcet of one function and some png call graphs
	#   macrocheck: see how macros are tested
	#   slip: (ms: will you write what it does?)
	#   tpiphost: host listener (UDP on serial 2)
	#   tpippatmos: patmos will send UDP over serial 2
	#   clean: removing fi
pcap:
	@echo 'tcpdump output:'
	#tcpdump -r tpip.pcap
	#tcpdump -nnvXSs 0 -c1 -i any

wcet:
	patmos-clang -O2 -mserialize=tpip.pml obb_demo.c tpip.c slip.c
	# patmos-clang -O0 -mserialize=tpip.pml tpip.c
	# the function to be analyzed in listed just before --report
	# functions: checksum waitfornextperiod wait
	# platin wcet -i tpip.pml -b a.out -e getipdstip  --report
	# platin wcet -i tpip.pml -b a.out -e wait --report
	platin wcet -i tpip.pml -b a.out -e packip
	platin wcet -i tpip.pml -b a.out -e tpip_slip_run
	#platin wcet -i tpip.pml -b a.out -e xmit
	#platin visualize -i tpip.pml -f waitfornextperiod
	# platin wcet -i tpip.pml -b a.out -e currenttimemillis --report
	# platin wcet -i tpip.pml -b a.out -e calculateipchecksum --report

#use this target to see how macros are expanded
macrocheck:
	$(CC) -E *.c
# Filter out unused files for tpip
LIBTPIP = $(filter-out $(LIBETH)/arp.c $(LIBETH)/icmp.c $(LIBETH)/ipv4.c $(LIBETH)/ptp1588.c $(LIBETH)/tcp.c $(LIBETH)/tte.c $(LIBETH)/udp.c, $(LIBETH)/*.c)

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
	patmos-ar r $@ $^

slip:
	$(CC) test_slip.c slip.c
	./a.out /dev/ttyUSB0

tpiphost:
    	#start listening on host
	$(CC) slip.c tpip.c test_tpip.c
	./a.out

tpippatmos:
	#start on patmos (a second terminal)
	patmos-clang $(CFLAGS) $(LIBETH)/*.c slip.c tpip_demo.c tpip.c -o tpip_obb.elf
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	patserdow -v /dev/ttyUSB0 tpip_obb.elf

tpipethpatmos:
	patmos-clang $(CFLAGS) $(LIBTPIP) tpip_arp.c demo.c tpip.c -o eth_demo.elf
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	patserdow -v /dev/ttyUSB0 eth_demo.elf

tcphost-comp:
	$(CC) -D $(TCP_ROLE) test_tcp_host.c -o test_tcp_host_$(TCP_ROLE).out

tcphost-run:
	./test_tcp_$(TCP_ROLE).out

tcppatmos-comp:
	patmos-clang -D $(TCP_ROLE) $(CFLAGS) $(LIBETH)/*.c test_tcp_patmos.c -o test_tcp_patmos_$(TCP_ROLE).elf

tcppatmos-run:
	cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
	sleep 2
	patserdow -v $(SERIAL) test_tcp_$(TCP_ROLE).elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf
