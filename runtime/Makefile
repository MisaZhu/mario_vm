ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

ifeq ($(MARIO_JS),)
MARIO_JS = ..
endif

NATIVE_PATH = builtin

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

NATIVE_OBJS= $(NATIVE_PATH)/basic/native_basic.o  \
	$(NATIVE_PATH)/math/native_math.o \
	$(NATIVE_PATH)/timer/native_timer.o 

OBJS = mario.o $(MARIO_JS)/mario_js.o \
		$(NATIVE_OBJS)

CFLAGS =  -g -I$(MARIO_JS) -Wall -fPIC -DMARIO_DEBUG -DMARIO_CACHE -DMARIO_THREAD
LDFLAGS =  -lm -ldl -lpthread

TARGET=mario
INST_DST=/usr/local/mario

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM

install:
	mkdir -p $(INST_DST)/bin
	cp $(TARGET) $(INST_DST)/bin
	mkdir -p $(INST_DST)/libs
	cp libs/* $(INST_DST)/libs
	mkdir -p $(INST_DST)/test.js/basic
	cp test.js/* $(INST_DST)/test.js/basic
