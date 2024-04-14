allocate: allocate.c allocate.h list.c list.h processManager.c processManager.h memoryManagement.c memoryManagement.h
	gcc -Wall -o allocate allocate.c list.c processManager.c memoryManagement.c


run:
	./allocate -f cases/task1/spec.txt -q 1 -m infinite
	
clean: 
	rm -f allocate allocate.o list.o processManager.o