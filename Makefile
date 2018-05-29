ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)g++

demo_SRCS = demo.c

CFLAGS =  -I./ -g -Wall

all: 
	$(CC) -o demo $(CFLAGS) $(demo_SRCS)


clean:
	rm -fr demo *.o *.dSYM arduino/esp8266/arduino_main/arduino_basic_native.h arduino/esp8266/arduino_main/mario.h
