SRCS = test.c
CFLAGS = -g -I./

all:
	gcc -o mario $(CFLAGS) $(SRCS)

clean:
	rm -fr $(OBJS) mario *.dSYM
