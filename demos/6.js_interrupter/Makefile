ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

ifeq ($(MARIO_VM),)
MARIO_VM = ../../vm
endif

mario_OBJS= $(MARIO_VM)/mario_utils.o $(MARIO_VM)/mario_bc.o $(MARIO_VM)/mario_vm.o $(MARIO_VM)/mario_lex.o
demo_OBJS = demo.o ../../lang/js/compiler/compiler.o 

CFLAGS = -I$(MARIO_VM) -I../.. -g -Wall -DMARIO_CACHE -DMARIO_DEBUG -DMARIO_THREAD
LDFLAGS = -lpthread

all: $(demo_OBJS) $(mario_OBJS)
	$(LD) -o demo $(demo_OBJS) $(mario_OBJS) $(LDFLAGS) 


clean:
	rm -fr demo $(demo_OBJS) $(mario_OBJS)  *.dSYM
