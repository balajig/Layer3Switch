#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

ARPS_CODE_DIR = $(CODE_DIR)/arp
ARPS_INC_DIR =  $(ARPS_CODE_DIR)/ -I$(COMMON_INC)
ARPS_SRCC_DIR = $(ARPS_CODE_DIR)

CFLAGS += -I$(ARPS_INC_DIR)

ARPS_SRCS = $(wildcard *.c)

ARP_OBJ = $(ARPS_SRCS:.c=.o)

all: $(ARP_OBJ) tm_arp.o

$(ARPS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_arp.o: $(ARPS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_arp.o $(ARPS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_arp.o
