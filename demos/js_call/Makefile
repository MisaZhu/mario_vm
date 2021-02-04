ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

MARIO_VM = ../..

mario_OBJS= $(MARIO_VM)/mario/mario.o

demo_OBJS = demo.o ../../lang/js/compiler.o 

CFLAGS = -I$(MARIO_VM)/mario -I../.. -g -Wall -DMARIO_CACHE -DMARIO_DEBUG -DMARIO_THREAD
LDFLAGS = -lpthread

all: $(demo_OBJS) $(mario_OBJS)
	$(LD) -o demo $(demo_OBJS) $(mario_OBJS) $(LDFLAGS) 


clean:
	rm -fr demo $(demo_OBJS) $(mario_OBJS)  *.dSYM
