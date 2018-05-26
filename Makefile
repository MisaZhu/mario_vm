ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)g++

demo_SRCS = demo.c
mario_SRCS = mario.c

CFLAGS =  -I./ -g -Wall

ifeq ($(CROSS_COMPILE), avr-)
CFLAGS =  -I./ -Os -DF_CPU=16000000UL -mmcu=atmega328p
endif

all: 
	$(CC) -o mario $(CFLAGS) $(mario_SRCS)
	$(CC) -o demo $(CFLAGS) $(demo_SRCS)


clean:
	rm -fr demo mario *.o *.dSYM
