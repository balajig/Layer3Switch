#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

IPV4_CODE_DIR = $(CODE_DIR)/ipv4
IPV4_INC_DIR =  $(IPV4_CODE_DIR)/ -I$(COMMON_INC)
IPV4_SRCC_DIR = $(IPV4_CODE_DIR)

CFLAGS += -I$(IPV4_INC_DIR)

IPV4_SRCS = $(wildcard *.c)

RTM_OBJ = $(IPV4_SRCS:.c=.o)

all: $(RTM_OBJ) ipv4.o

$(IPV4_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
ipv4.o: $(IPV4_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/ipv4.o $(IPV4_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/ipv4.o
