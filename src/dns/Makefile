#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

DNSS_CODE_DIR = $(CODE_DIR)/dns
DNSS_INC_DIR =  $(DNSS_CODE_DIR)/ -I$(COMMON_INC)
DNSS_SRCC_DIR = $(DNSS_CODE_DIR)

CFLAGS += -I$(DNSS_INC_DIR)

DNSS_SRCS = $(wildcard *.c)

DNS_OBJ = $(DNSS_SRCS:.c=.o)

all: $(DNS_OBJ) tm_dns.o

$(DNSS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_dns.o: $(DNSS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_dns.o $(DNSS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_dns.o
