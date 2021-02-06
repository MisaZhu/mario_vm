ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

ifeq ($(MARIO_LANG),)
MARIO_LANG = js
endif

ifeq ($(MARIO_VM),)
MARIO_VM = ../../../mario
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)g++

mario_OBJS= $(MARIO_VM)/mario.o 

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

OBJS= $(mario_OBJS) \
	native_demo.o 


CFLAGS =  -I$(MARIO_VM) -g -Wall -DMARIO_DEBUG -DMARIO_CACHE -DMARIO_THREAD -fPIC
LDFLAGS =  -lm -lpthread

NAME=demo
TARGET=native_$(NAME).so
INST_DST=/usr/local/mario

all: $(OBJS)
	$(LD) -shared -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM

install:
	mkdir -p $(INST_DST)/natives
	cp $(TARGET) $(INST_DST)/natives
	mkdir -p $(INST_DST)/test/$(MARIO_LANG)/$(NAME)
	cp test/$(MARIO_LANG)/* $(INST_DST)/test/$(MARIO_LANG)/$(NAME)
