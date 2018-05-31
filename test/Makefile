ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)g++

test_SRCS = jstest.c

CFLAGS =  -I../ -g -Wall

all: 
	$(CC) -o jstest $(CFLAGS) $(test_SRCS)
	$(CC) -o jsverify $(CFLAGS) -D VERIFY -D MARIO_DEBUG $(test_SRCS)


clean:
	rm -fr jstest jsverify *.o *.dSYM
