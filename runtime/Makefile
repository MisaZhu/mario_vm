ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

ifeq ($(MARIO_JS),)
MARIO_JS = ..
endif

NATIVE_PATH = natives

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

NATIVE_OBJS= $(NATIVE_PATH)/basic/native_basic.o  \
	$(NATIVE_PATH)/math/native_math.o 

OBJS = mario.o $(MARIO_JS)/mario_js.o \
		$(NATIVE_OBJS)

CFLAGS =  -I$(MARIO_JS) -Wall -fPIC -DMARIO_DEBUG -DMARIO_CACHE
LDFLAGS =  -lm -ldl

#ifeq ($(MARIO_THREAD), yes)
CFLAGS +=  -DMARIO_THREAD
LDFLAGS += -lpthread
#endif



all: $(OBJS)
	$(LD) -o mario $(OBJS) $(LDFLAGS)

clean:
	rm -fr mario $(OBJS) *.dSYM
