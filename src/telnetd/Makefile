#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

TELNETD_CODE_DIR = $(CODE_DIR)/telnetd
TELNETD_INC_DIR =  $(TELNETD_CODE_DIR)/ -I$(COMMON_INC)
TELNETD_SRCC_DIR = $(TELNETD_CODE_DIR)

TELNETD_SRCS += $(TELNETD_SRCC_DIR)/telnet.c
TELNETD_SRCS += $(TELNETD_SRCC_DIR)/telnet_client.c
TELNETD_SRCS += $(TELNETD_SRCC_DIR)/telnet_cli.c

CFLAGS += -I$(TELNETD_INC_DIR)

LIB_OBJ = $(TELNETD_SRCS:.c=.o)

all: $(LIB_OBJ) tm_telnet.o

$(TELNETD_OBJ) : %.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_telnet.o: $(TELNETD_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_telnet.o $(TELNETD_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_telnet.o
