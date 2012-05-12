#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

OSPF4_CODE_DIR = $(CODE_DIR)/ospf
OSPF4_INC_DIR =  $(OSPF4_CODE_DIR)/ -I$(COMMON_INC)
OSPF4_SRCC_DIR = $(OSPF4_CODE_DIR)

CFLAGS += -I$(OSPF4_INC_DIR)

OSPF4_SRCS = $(wildcard *.c)

OSPF_OBJ = $(OSPF4_SRCS:.c=.o)

all: $(OSPF_OBJ) ospf.o

$(OSPF4_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
ospf.o: $(OSPF4_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/ospf.o $(OSPF4_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/ospf.o
