allocate: allocate.c allocate.h list.c list.h processManager.c processManager.h
	gcc -Wall -o allocate allocate.c list.c processManager.c


run:
	./allocate -f cases/task1/spec.txt -q 1 -m infinite
