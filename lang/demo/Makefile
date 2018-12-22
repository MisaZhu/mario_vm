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

lang_OBJS = compiler.o

OBJS = demo.o $(mario_OBJS) $(lang_OBJS)

CFLAGS =  -g -I$(MARIO_VM) -I$(MARIO_UTILS) -Wall -fPIC -DMARIO_DEBUG -DMARIO_CACHE -DMARIO_THREAD
LDFLAGS =  -lm -ldl -lpthread

TARGET=demo
INST_DST=/usr/local/mario

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM

install:
	mkdir -p $(INST_DST)/bin
	cp $(TARGET) $(INST_DST)/bin
