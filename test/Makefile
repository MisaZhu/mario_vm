ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

ifeq ($(MARIO_LANG),)
MARIO_LANG = js
endif

ifeq ($(MARIO_VM),)
MARIO_VM = ..
endif
MARIO_COMP = $(MARIO_VM)/compilers/$(MARIO_LANG)

NATIVE_PATH = builtin

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

mario_OBJS= $(MARIO_VM)/mario_utils.o $(MARIO_VM)/mario_bc.o $(MARIO_VM)/mario_vm.o $(MARIO_COMP)/compiler.o
OBJS = mariotest.o $(mario_OBJS)

CFLAGS =  -g -I$(MARIO_VM) -I$(MARIO_COMP) -Wall -fPIC -DMARIO_DEBUG -DMARIO_CACHE -DMARIO_THREAD
LDFLAGS =  -lm -ldl -lpthread

TARGET=mariotest
INST_DST=/usr/local/mario

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM

install:
	mkdir -p $(INST_DST)/bin
	cp $(TARGET) $(INST_DST)/bin
