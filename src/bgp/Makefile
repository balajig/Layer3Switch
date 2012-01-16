#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

BGPS_CODE_DIR = $(CODE_DIR)/bgp
BGPS_INC_DIR =  $(BGPS_CODE_DIR)/ -I$(COMMON_INC)
BGPS_SRCC_DIR = $(BGPS_CODE_DIR)

CFLAGS += -I$(BGPS_INC_DIR)

BGPS_SRCS = $(wildcard *.c)

BGP_OBJ = $(BGPS_SRCS:.c=.o)

all: $(BGP_OBJ) bgp_d.o

$(BGPS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
bgp_d.o: $(BGPS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/bgp_d.o $(BGPS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/bgp_d.o
