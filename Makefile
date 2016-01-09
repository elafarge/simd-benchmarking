

all: prepare simdbmk

simdbmk: gcc_build/utilities.o gcc_build/cli_arguments.o gcc_build/simdbmk.o
	gcc -std=c99 -pthread -o gcc_build/simdbmk gcc_build/utilities.o \
		                                       gcc_build/simdbmk.o \
											   gcc_build/cli_arguments.o

gcc_build/simdbmk.o: simdbmk.c
	gcc -std=c99 -mavx2 -o gcc_build/simdbmk.o -c simdbmk.c

gcc_build/cli_arguments.o: cli_arguments.c
	gcc -std=c99 -o gcc_build/cli_arguments.o -c cli_arguments.c

gcc_build/utilities.o: utilities.c
	gcc -std=c99 -o gcc_build/utilities.o -c utilities.c

prepare:
	mkdir -p ./gcc_build

clean:
	rm gcc_build/*
	rm gcc_build/simdbmk
