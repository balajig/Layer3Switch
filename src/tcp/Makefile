#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

TCPS_CODE_DIR = $(CODE_DIR)/tcp
TCPS_INC_DIR =  $(TCPS_CODE_DIR)/ -I$(COMMON_INC)
TCPS_SRCC_DIR = $(TCPS_CODE_DIR)

CFLAGS += -I$(TCPS_INC_DIR)

TCPS_SRCS = $(wildcard *.c)

TCP_OBJ = $(TCPS_SRCS:.c=.o)

all: $(TCP_OBJ) tm_tcp.o

$(TCPS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_tcp.o: $(TCPS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_tcp.o $(TCPS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_tcp.o
