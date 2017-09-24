KANOODLE_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wswitch -Wwrite-strings

kanoodle: kanoodle.o
	gcc -o kanoodle kanoodle.o

kanoodle.o: kanoodle.c kanoodle.make
	gcc ${KANOODLE_C_FLAGS} -o kanoodle.o kanoodle.c

clean:
	rm -f kanoodle kanoodle.o
