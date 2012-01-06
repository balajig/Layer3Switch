#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

INIT_CODE_DIR = $(CODE_DIR)/init
INIT_INC_DIR =  $(INIT_CODE_DIR)/ -I$(COMMON_INC)
INIT_SRCC_DIR = $(INIT_CODE_DIR)

CFLAGS += -I$(INIT_INC_DIR)

INIT_SRCS = $(wildcard *.c)

INIT_OBJ = $(INIT_SRCS:.c=.o)

all: $(INIT_OBJ) InitSystem.o

$(INIT_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
InitSystem.o: $(INIT_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/InitSystem.o $(INIT_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/InitSystem.o
