ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

ifeq ($(MARIO_JS),)
MARIO_JS = ../..
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

OBJS= $(MARIO_JS)/mario_js.o \
	native_demo.o 


CFLAGS =  -I$(MARIO_JS) -g -Wall -DMARIO_DEBUG -DMARIO_CACHE -DMARIO_THREAD -fPIC
LDFLAGS =  -lm -lpthread

NAME=demo
TARGET=native_$(NAME).so
INST_DST=/usr/local/mario

all: $(OBJS)
	$(LD) -shared -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM

install:
	mkdir -p $(INST_DST)/libs
	cp $(TARGET) $(INST_DST)/libs
	mkdir -p $(INST_DST)/test.js/$(NAME)
	cp test.js/* $(INST_DST)/test.js/$(NAME)
