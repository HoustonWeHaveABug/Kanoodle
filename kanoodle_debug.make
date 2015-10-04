KANOODLE_DEBUG_C_FLAGS=-c -g -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wswitch -Wwrite-strings

kanoodle_debug.exe: kanoodle_debug.o
	gcc -g -o kanoodle_debug.exe kanoodle_debug.o

kanoodle_debug.o: kanoodle.c kanoodle_debug.make
	gcc ${KANOODLE_DEBUG_C_FLAGS} -o kanoodle_debug.o kanoodle.c
