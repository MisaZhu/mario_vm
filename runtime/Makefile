ifneq ($(CROSS_COMPILE),)
$(info CROSS_COMPILE=$(CROSS_COMPILE))
endif

MARIO_LANG = js

ifeq ($(MARIO_VM),)
MARIO_VM = ..
endif
MARIO_COMP = $(MARIO_VM)/lang/$(MARIO_LANG)


CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)gcc

NATIVE_PATH = $(MARIO_VM)/lang/js/native/builtin
NATIVE_OBJS= $(NATIVE_PATH)/native_builtin.o  \
	$(NATIVE_PATH)/object/native_object.o  \
	$(NATIVE_PATH)/number/native_number.o  \
	$(NATIVE_PATH)/bytes/native_bytes.o  \
	$(NATIVE_PATH)/console/native_console.o \
	$(NATIVE_PATH)/string/native_string.o \
	$(NATIVE_PATH)/array/native_array.o \
	$(NATIVE_PATH)/math/native_math.o \
	$(NATIVE_PATH)/json/native_json.o

mario_OBJS= $(MARIO_VM)/utils/mario_basic.o \
						$(MARIO_VM)/utils/mario_array.o \
						$(MARIO_VM)/utils/mario_string.o \
						$(MARIO_VM)/utils/mario_utf8.o \
						$(MARIO_VM)/vm/mario_json.o \
						$(MARIO_VM)/vm/mario_bc.o \
						$(MARIO_VM)/vm/mario_vm.o \
						$(MARIO_VM)/vm/mario_lex.o

lang_OBJS = $(MARIO_COMP)/compiler.o 
OBJS = shell/mario.o shell/shell.o $(mario_OBJS) $(lang_OBJS) \
		$(NATIVE_OBJS)

CFLAGS = -I$(NATIVE_PATH) -I$(MARIO_VM)/utils -I$(MARIO_VM)/vm -I$(MARIO_COMP) -Wall -fPIC

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


LDFLAGS =  -lm -ldl -lpthread

TARGET_PATH=lang/$(MARIO_LANG)
TARGET=mario
INST_DST=/usr/local/mario

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -fr $(TARGET) $(OBJS) *.dSYM

install:
	mkdir -p $(INST_DST)/bin
	cp $(TARGET) $(INST_DST)/bin
	mkdir -p $(INST_DST)/test/$(MARIO_LANG)
	cp $(TARGET_PATH)/test/* $(INST_DST)/test/$(MARIO_LANG)
