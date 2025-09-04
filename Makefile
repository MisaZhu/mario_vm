CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

ifeq ($(MARIO_VM),)
MARIO_VM = .
endif

include $(MARIO_VM)/lang/js/lang.mk

mario_OBJS = $(MARIO_VM)/mario/mario.o $(MARIO_VM)/mario/bcdump/bcdump.o
platform_OBJS = $(MARIO_VM)/platform/platform.o \
		$(MARIO_VM)/platform/mem.o
mvm_OBJS = bin/mario/main.o bin/lib/mbc.o bin/lib/js.o 

MARIO_OBJS = $(mario_OBJS) $(mvm_OBJS) $(lang_OBJS) $(platform_OBJS) \
		$(NATIVE_OBJS)

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
endif


HEADS = -I$(NATIVE_PATH_BUILTIN) \
	-I$(MARIO_VM)/mario \
	-I$(MARIO_VM)/bin/lib \
	-I$(MARIO_VM)/platform

CFLAGS += $(HEADS)
CXXFLAGS += $(HEADS)

CFLAGS += -Wno-incompatible-function-pointer-types
CXXFLAGS += -Wno-incompatible-function-pointer-types

MARIO = build/mario

all: $(MARIO)
	@echo "done"


$(MARIO): $(MARIO_OBJS)
	mkdir -p build
	$(LD) -o $(MARIO) $(MARIO_OBJS) $(LDFLAGS)

clean:
	rm -f $(MARIO_OBJS) $(MARIO)
