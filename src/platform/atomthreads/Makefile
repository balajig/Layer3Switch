#   $Id: Makefile,v 1.4 2010/10/25 20:48:19 Sasi Exp $ 
.PHONY: all all-before all-after clean clean-custom

include ../../build/make.h

ATOMS_CODE_DIR = $(CODE_DIR)/platform/atomthreads
HAL_CODE_DIR = $(CODE_DIR)/platform/hal
ATOMS_INC_DIR =  $(ATOMS_CODE_DIR)/inc -I$(COMMON_INC)
ATOMS_SRCC_DIR = $(ATOMS_CODE_DIR)/src
ATOMS_OBJ_DIR = $(ATOMS_CODE_DIR)/obj

ATOMS_OBJS += $(ATOMS_OBJ_DIR)/main_sim.o
ATOMS_OBJS += $(ATOMS_OBJ_DIR)/tm_virtl.o
ATOMS_OBJS += $(ATOMS_OBJ_DIR)/hal_cli.o
ATOMS_OBJS += $(ATOMS_OBJ_DIR)/hal.o
ATOMS_OBJS += $(ATOMS_OBJ_DIR)/port.o

all: all-before  obj $(ATOMS_OBJS) atomthreads.o all-after

obj:
	mkdir -p ${ATOMS_OBJ_DIR}

clean: clean-custom
	$(RM) $(ATOMS_OBJ_DIR)/*.o $(FINAL_OBJ)/../atomthreads_obj/atomthreads.o

$(ATOMS_OBJ_DIR)/main_sim.o: $(ATOMS_SRCC_DIR)/main_sim.c
	$(CC) -c $(COMPILATION_SWITCH) $(ATOMS_SRCC_DIR)/main_sim.c -o $(ATOMS_OBJ_DIR)/main_sim.o $(CFLAGS) -I$(ATOMS_INC_DIR)

$(ATOMS_OBJ_DIR)/tm_virtl.o: $(ATOMS_SRCC_DIR)/tm_virtl.c
	$(CC) -c $(COMPILATION_SWITCH) $(ATOMS_SRCC_DIR)/tm_virtl.c -o $(ATOMS_OBJ_DIR)/tm_virtl.o $(CFLAGS) -I$(ATOMS_INC_DIR)

$(ATOMS_OBJ_DIR)/port.o: $(ATOMS_SRCC_DIR)/port.c
	$(CC) -c $(COMPILATION_SWITCH) $(ATOMS_SRCC_DIR)/port.c -o $(ATOMS_OBJ_DIR)/port.o $(CFLAGS) -I$(ATOMS_INC_DIR)

$(ATOMS_OBJ_DIR)/hal_cli.o: $(HAL_CODE_DIR)/hal_cli.c
	$(CC) -c $(COMPILATION_SWITCH) $(HAL_CODE_DIR)/hal_cli.c -o $(ATOMS_OBJ_DIR)/hal_cli.o $(CFLAGS) -I$(ATOMS_INC_DIR)

$(ATOMS_OBJ_DIR)/hal.o: $(HAL_CODE_DIR)/hal.c
	$(CC) -c $(COMPILATION_SWITCH) $(HAL_CODE_DIR)/hal.c -o $(ATOMS_OBJ_DIR)/hal.o $(CFLAGS) -I$(ATOMS_INC_DIR)

atomthreads.o: $(ATOMS_OBJS)
	$(LD) $(LD_FLAGS) -o $(FINAL_OBJ)/../atomthreads_obj/atomthreads.o $(ATOMS_OBJS)
