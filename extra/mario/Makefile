ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

ifeq ($(TARGET),)
#TARGET = nil
TARGET = posix
endif

MARIO_PATH = ../..
HAI_NATIVE_PATH = ../native
HAI_PATH = ../../../mario_hai

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

HAI_NATIVE_OBJS= $(HAI_NATIVE_PATH)/basic/native_basic.o \
	$(HAI_NATIVE_PATH)/fs/native_fs.o \
	$(HAI_NATIVE_PATH)/debug/native_debug.o

HAI_OBJS= $(HAI_PATH)/platforms/hai.o \
	$(HAI_PATH)/platforms/$(TARGET)/hai_fs.o  \
	$(HAI_PATH)/platforms/$(TARGET)/hai_debug.o 
	

OBJS = mario.o $(MARIO_PATH)/mario_js.o \
		$(HAI_NATIVE_OBJS) \
		$(HAI_OBJS)

CFLAGS =  -I$(HAI_PATH)/include -I$(HAI_NATIVE_PATH) -I$(MARIO_PATH) -g -Wall
LDFLAGS = 

all: $(OBJS)
	$(LD) -o mario $(OBJS) $(LDFLAGS)

clean:
	rm -fr mario $(OBJS) *.dSYM
