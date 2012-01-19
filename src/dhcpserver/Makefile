#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

DHCPS_CODE_DIR = $(CODE_DIR)/dhcpserver
DHCPS_INC_DIR =  $(DHCPS_CODE_DIR)/ -I$(COMMON_INC)
DHCPS_SRCC_DIR = $(DHCPS_CODE_DIR)

CFLAGS += -I$(DHCPS_INC_DIR)

DHCPS_SRCS = $(wildcard *.c)

DHCP_OBJ = $(DHCPS_SRCS:.c=.o)

all: $(DHCP_OBJ) dhcpserver.o

$(DHCPS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
dhcpserver.o: $(DHCPS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/dhcpserver.o $(DHCPS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/dhcpserver.o
