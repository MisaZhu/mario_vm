ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

#MARIO_GUI = yes

ifeq ($(MARIO_JS),)
MARIO_JS = ..
endif

NATIVE_PATH = native_builtin

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

NATIVE_OBJS= $(NATIVE_PATH)/basic/native_basic.o \
	$(NATIVE_PATH)/math/native_math.o 

OBJS = mario.o $(MARIO_JS)/mario_js.o \
		$(NATIVE_OBJS)

CFLAGS =  -I$(NATIVE_PATH) -I$(MARIO_JS) -g -Wall -DMARIO_DEBUG -DMARIO_CACHE
LDFLAGS =  -lm -ldl

ifeq ($(MARIO_GUI), yes)
NATIVE_OBJS += $(NATIVE_PATH)/gui/native_gui.o
CFLAGS += -DMARIO_GUI -I/usr/local/include/SDL2
LDFLAGS += -L/usr/local/lib -lSDL2 -lSDL2_gfx -lSDL2_image -lSDL2_ttf
endif


all: $(OBJS)
	$(LD) -o mario $(OBJS) $(LDFLAGS)

clean:
	rm -fr mario $(OBJS) *.dSYM
