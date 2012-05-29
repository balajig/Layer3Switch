#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

TRACERTS_CODE_DIR = $(CODE_DIR)/traceroute
TRACERTS_INC_DIR =  $(TRACERTS_CODE_DIR)/ -I$(COMMON_INC)
TRACERTS_SRCC_DIR = $(TRACERTS_CODE_DIR)

CFLAGS += -I$(TRACERTS_INC_DIR)

TRACERTS_SRCS = $(wildcard *.c)

TRACERT_OBJ = $(TRACERTS_SRCS:.c=.o)

all: $(TRACERT_OBJ) tm_tracert.o

$(TRACERTS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_tracert.o: $(TRACERTS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_tracert.o $(TRACERTS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_tracert.o
