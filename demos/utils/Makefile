ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

MARIO_UTILS = ../../utils

mario_OBJS= $(MARIO_UTILS)/mario_basic.o \
						$(MARIO_UTILS)/mario_string.o \
						$(MARIO_UTILS)/mario_array.o \
						$(MARIO_UTILS)/mario_utf8.o

demo_OBJS = demo.o

CFLAGS = -I$(MARIO_UTILS) -I../.. -g -Wall -DMARIO_CACHE -DMARIO_DEBUG -DMARIO_THREAD
LDFLAGS = -lpthread

TARGET=demo
INST_DST=/usr/local/mario

all: $(demo_OBJS) $(mario_OBJS)
	$(LD) -o $(TARGET) $(demo_OBJS) $(mario_OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(demo_OBJS) $(mario_OBJS) *.dSYM

install:
	mkdir -p $(INST_DST)/bin
	cp $(TARGET) $(INST_DST)/bin
