#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

SNTP_CODE_DIR = $(CODE_DIR)/sntp
SNTP_INC_DIR =  $(SNTP_CODE_DIR)/ -I$(COMMON_INC)
SNTP_SRCC_DIR = $(SNTP_CODE_DIR)

CFLAGS += -I$(SNTP_INC_DIR)

SNTP_SRCS = $(wildcard *.c)

RTM_OBJ = $(SNTP_SRCS:.c=.o)

all: $(RTM_OBJ) tm_sntp.o

$(SNTP_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_sntp.o: $(SNTP_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_sntp.o $(SNTP_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_sntp.o
