

all: prepare simdbmk

simdbmk: gcc_build/utilities.o gcc_build/cli_arguments.o gcc_build/find.o \
	     gcc_build/thread_find.o gcc_build/main.o
	gcc -std=c99 -pthread -o gcc_build/simdbmk gcc_build/utilities.o \
				   			                   gcc_build/cli_arguments.o \
				   			                   gcc_build/find.o \
				   			                   gcc_build/thread_find.o \
		                                       gcc_build/main.o

gcc_build/main.o: main.c
	gcc -std=c99 -o gcc_build/main.o -c main.c

gcc_build/thread_find.o: thread_find.c
	gcc -std=c99 -o gcc_build/thread_find.o -c thread_find.c

gcc_build/find.o: find.c
	gcc -std=c99 -mavx2 -o gcc_build/find.o -c find.c

gcc_build/cli_arguments.o: cli_arguments.c
	gcc -std=c99 -o gcc_build/cli_arguments.o -c cli_arguments.c

gcc_build/utilities.o: utilities.c
	gcc -std=c99 -o gcc_build/utilities.o -c utilities.c

prepare:
	mkdir -p ./gcc_build

clean:
	rm gcc_build/*
