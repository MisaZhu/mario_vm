ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

MARIO_LANG = js
MARIO_VM = ../../vm
MARIO_UTILS = ../../utils

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

mario_OBJS= $(MARIO_UTILS)/mario_basic.o \
						$(MARIO_UTILS)/mario_string.o \
						$(MARIO_UTILS)/mario_array.o \
						$(MARIO_VM)/mario_bc.o \
						$(MARIO_VM)/mario_vm.o \
						$(MARIO_VM)/mario_lex.o

lang_OBJS = compiler.o 
OBJS = jsdemo.o $(mario_OBJS) $(lang_OBJS)

CFLAGS = -I$(MARIO_UTILS) -I$(MARIO_VM) -Wall -fPIC

ifneq ($(MARIO_DEBUG), no)
CFLAGS += -g -DMARIO_DEBUG
else
CFLAGS += -O2
endif

ifneq ($(MARIO_CACHE), no)
CFLAGS += -DMARIO_CACHE
endif

ifneq ($(MARIO_THREAD), no)
CFLAGS += -DMARIO_THREAD
endif


LDFLAGS =  -lm -ldl -lpthread

TARGET_PATH=lang/$(MARIO_LANG)
TARGET=jsdemo
INST_DST=/usr/local/mario

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM

install:
	mkdir -p $(INST_DST)/bin
	cp $(TARGET) $(INST_DST)/bin
	mkdir -p $(INST_DST)/test/$(MARIO_LANG)
	cp $(TARGET_PATH)/test/* $(INST_DST)/test/$(MARIO_LANG)/
