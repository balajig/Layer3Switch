#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

UTILS_CODE_DIR = $(CODE_DIR)/utils
UTILS_INC_DIR =  $(UTILS_CODE_DIR)/ -I$(COMMON_INC)
UTILS_SRCC_DIR = $(UTILS_CODE_DIR)

CFLAGS += -I$(UTILS_INC_DIR)

UTILS_SRCS = $(wildcard *.c)

UTIL_OBJ = $(UTILS_SRCS:.c=.o)

all: $(UTIL_OBJ) tm_utils.o

$(UTILS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
tm_utils.o: $(UTILS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/tm_utils.o $(UTILS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/tm_utils.o
