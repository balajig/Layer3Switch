#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

ROUTE_CODE_DIR = $(CODE_DIR)/route
ROUTE_INC_DIR =  $(ROUTE_CODE_DIR)/ -I$(COMMON_INC)
ROUTE_SRCC_DIR = $(ROUTE_CODE_DIR)

ROUTE_SRCS += $(ROUTE_SRCC_DIR)/route.c
ROUTE_SRCS += $(ROUTE_SRCC_DIR)/route_cli.c

CFLAGS += -I$(ROUTE_INC_DIR)

ROUTE_OBJ = $(ROUTE_SRCS:.c=.o)

all: $(ROUTE_OBJ) tm_route.o

$(ROUTE_OBJ) : %.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_route.o: $(ROUTE_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_route.o $(ROUTE_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_route.o
