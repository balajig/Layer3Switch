#Makefile for Lib

obj-$(CONFIG_GEN_TIMERS) += tmr_init.o
obj-$(CONFIG_GEN_TIMERS) += update.o
obj-$(CONFIG_GEN_TIMERS) += thr_sp.o.o
obj-$(CONFIG_GEN_TIMERS) += btm_hlf.o

obj-$(CONFIG_LIGHT_WEIGHT_TIMERS) += lwtimers.o
obj-y += libinit.o
obj-y += util.o
obj-y += msg_queue.o
obj-y += hashlib.o
obj-y += rbtree.o
obj-y += mem_main.o
obj-y += tsk_main.o
obj-y += memp.o
obj-y += mem.o
obj-y += pbuf.o
obj-y += def.o
obj-y += table.o
obj-y += linklist.o
obj-y += prefix.o
obj-y += v.otor.o
obj-y += hash.o
obj-y += pqueue.o
obj-y += libtelnet.o


CFLAGS += -I$(LIBS_INC_DIR) -DTIMER_BTM_HALF

