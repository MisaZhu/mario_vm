ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

MARIO_VM = ../../vm
MARIO_UTILS = ../../utils

mario_OBJS= $(MARIO_UTILS)/mario_basic.o \
						$(MARIO_UTILS)/mario_string.o \
						$(MARIO_UTILS)/mario_array.o \
						$(MARIO_VM)/mario_bc.o \
						$(MARIO_VM)/mario_vm.o \
						$(MARIO_VM)/mario_lex.o

demo_OBJS = demo.o ../../lang/js/compiler.o 

CFLAGS = -I$(MARIO_VM) -I$(MARIO_UTILS) -I../.. -g -Wall -DMARIO_CACHE -DMARIO_DEBUG -DMARIO_THREAD
LDFLAGS = -lpthread

all: $(demo_OBJS) $(mario_OBJS)
	$(LD) -o demo $(demo_OBJS) $(mario_OBJS) $(LDFLAGS) 


clean:
	rm -fr demo $(demo_OBJS) $(mario_OBJS)  *.dSYM
