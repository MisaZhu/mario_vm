all:
	gcc -g -o mario *.c -I./

clean:
	rm -fr mario *.dSYM
