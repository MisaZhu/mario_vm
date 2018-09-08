ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif


ifeq ($(MARIO_JS),)
MARIO_JS = ../../../..
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

OBJS= $(MARIO_JS)/mario_js.o  \
	native_sys.o \
	system/native_system.o \
	socket/native_socket.o  \
	fs/native_fs.o


CFLAGS =  -I$(MARIO_JS) -g -Wall -DMARIO_DEBUG -DMARIO_CACHE
LDFLAGS = 

TARGET=native_sys.so

all: $(OBJS)
	$(LD) -shared -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM
