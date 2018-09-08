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
	sdl/native_sdl.o \
	native_gui.o 

CFLAGS =  -I$(MARIO_JS) -g -Wall -DMARIO_DEBUG -DMARIO_CACHE -I/usr/local/include/SDL2
LDFLAGS = -L/usr/local/lib -lSDL2 -lSDL2_gfx -lSDL2_image -lSDL2_ttf

TARGET=native_gui.so

all: $(OBJS)
	$(LD) -shared -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM
