ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

mario_OBJS= ../mario_js.o
demo_OBJS = demo.o 

CFLAGS = -I.. -g -Wall -DMARIO_CACHE -DMARIO_DEBUG -DMARIO_THREAD
LDFLAGS = -lpthread
#CFLAGS = -I.. -O2 -Wall -DMARIO_CACHE -DMARIO_DEBUG  -DMARIO_THREAD

all: $(demo_OBJS) $(mario_OBJS)
	$(LD) -o demo $(LDFLAGS) $(demo_OBJS) $(mario_OBJS)


clean:
	rm -fr demo *.o *.dSYM
