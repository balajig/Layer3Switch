#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../build/make.h

RTMS_CODE_DIR = $(CODE_DIR)/rtm
RTMS_INC_DIR =  $(RTMS_CODE_DIR)/ -I$(COMMON_INC)
RTMS_SRCC_DIR = $(RTMS_CODE_DIR)

CFLAGS += -I$(RTMS_INC_DIR)

RTMS_SRCS = $(wildcard *.c)

RTM_OBJ = $(RTMS_SRCS:.c=.o)

all: $(RTM_OBJ) rtm.o

$(RTMS_OBJ)/%.o : %.c Makefile
	$(CC) -c $(CFLAGS) $< -o $@
rtm.o: $(RTMS_OBJ)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/rtm.o $(RTMS_SRCC_DIR)/*.o

clean:
	rm -f *.o $(FINAL_OBJ)/rtm.o
