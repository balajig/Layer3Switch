.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

DHCPS_CODE_DIR = $(CODE_DIR)/dhcp
DHCPS_INC_DIR =  $(DHCPS_CODE_DIR)/ -I$(COMMON_INC)
DHCPS_SRCC_DIR = $(DHCPS_CODE_DIR)

CFLAGS += -I$(DHCPS_INC_DIR)

DHCPS_SRCS = $(wildcard *.c)

DHCP_OBJ = $(DHCPS_SRCS:.c=.o)

all: $(DHCP_OBJ) tm_dhcp.o

$(DHCPS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_dhcp.o: $(DHCPS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_dhcp.o $(DHCPS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_dhcp.o
