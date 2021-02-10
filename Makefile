ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

ifeq ($(MARIO_VM),)
MARIO_VM = .
endif

CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

include $(MARIO_VM)/lang/js/lang.mk

mario_OBJS= $(MARIO_VM)/mario/mario.o

OBJS = shell/mariovm.o shell/shell.o $(mario_OBJS) $(lang_OBJS) \
		$(NATIVE_OBJS)

CFLAGS = -I$(NATIVE_PATH) -I$(MARIO_VM)/mario -Wall -fPIC

ifneq ($(MARIO_DEBUG), no)
CFLAGS += -g -DMARIO_DEBUG
else
CFLAGS += -O2
endif

ifneq ($(MARIO_CACHE), no)
CFLAGS += -DMARIO_CACHE
endif

ifneq ($(MARIO_THREAD), no)
CFLAGS += -DMARIO_THREAD
LDFLAGS +=  -lpthread
endif

LDFLAGS += -lm -ldl

TARGET_PATH=lang/$(MARIO_LANG)
TARGET=mariovm
INST_DST=/usr/local/mario

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM

install:
	cp $(TARGET) /usr/local/bin
	mkdir -p $(INST_DST)/test/$(MARIO_LANG)
	cp test/$(MARIO_LANG)/* $(INST_DST)/test/$(MARIO_LANG)
