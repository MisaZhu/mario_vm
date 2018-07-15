ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

test_OBJS = jstest.o mario_js.o

CFLAGS =  -g -Wall -DMARIO_DEBUG -DVAR_CACHE
#CFLAGS = -Wall -DMARIO_DEBUG

all: $(test_OBJS)
	$(LD) -o jstest $(LDFLAGS) $(test_OBJS)


clean:
	rm -fr jstest $(test_OBJS) *.dSYM
