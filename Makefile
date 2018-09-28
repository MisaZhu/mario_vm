ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

mario_OBJS=mario_js.o
test_OBJS = jstest.o 

CFLAGS = -g -Wall -DMARIO_CACHE -DMARIO_DEBUG -DMARIO_THREAD
#CFLAGS = -O2 -Wall -DMARIO_CACHE -DMARIO_DEBUG  -DMARIO_THREAD

all: $(test_OBJS) $(mario_OBJS)
	$(LD) -o jstest $(LDFLAGS) $(test_OBJS) $(mario_OBJS)


clean:
	rm -fr jstest *.o *.dSYM
