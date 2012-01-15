#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

SNMP_CODE_DIR = $(CODE_DIR)/snmp
SNMP_INC_DIR =  $(SNMP_CODE_DIR)/ -I$(COMMON_INC)
SNMP_SRCC_DIR = $(SNMP_CODE_DIR)

CFLAGS += -I$(SNMP_INC_DIR)

SNMP_SRCS = $(wildcard *.c)

RTM_OBJ = $(SNMP_SRCS:.c=.o)

all: $(RTM_OBJ) tm_snmp.o

$(SNMP_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_snmp.o: $(SNMP_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_snmp.o $(SNMP_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_snmp.o
