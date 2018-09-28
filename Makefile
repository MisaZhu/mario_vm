ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

mario_OBJS=mario_js.o
test_OBJS = jstest.o 

CFLAGS = -g -Wall -DMARIO_CACHE -DMARIO_DEBUG
LDFLAGS =
#CFLAGS = -O2 -Wall -DMARIO_CACHE -DMARIO_DEBUG

ifeq ($(MARIO_THREAD), yes)
CFLAGS +=  -DMARIO_THREAD
LDFLAGS += -lpthread
endif

all: $(test_OBJS) $(mario_OBJS)
	$(LD) -o jstest $(test_OBJS) $(mario_OBJS) $(LDFLAGS)


clean:
	rm -fr jstest *.o *.dSYM
