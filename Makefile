allocate: allocate.c allocate.h list.c list.h processManager.c processManager.h memoryManagement.c memoryManagement.h
	gcc -Wall -o allocate allocate.c list.c processManager.c memoryManagement.c


run:
	./allocate -f cases/task1/spec.txt -q 1 -m infinite | diff - cases/task1/spec-q1.out
	./allocate -f cases/task1/two-processes.txt -q 1 -m infinite | diff - cases/task1/two-processes-q1.out
	./allocate -f cases/task1/two-processes.txt -q 3 -m infinite | diff - cases/task1/two-processes-q3.out

	./allocate -f cases/task3/simple-alloc.txt -q 3 -m paged | diff - cases/task3/simple-alloc-q3.out
	./allocate -f cases/task3/simple-evict.txt -q 1 -m paged | diff - cases/task3/simple-evict-q1.out
	./allocate -f cases/task3/internal-frag.txt -q 1 -m paged | diff - cases/task3/internal-frag-q1.out

	./allocate -f cases/task4/no-evict.txt -q 3 -m virtual | diff - cases/task4/no-evict-q3.out
	./allocate -f cases/task4/virtual-evict.txt -q 1 -m virtual | diff - cases/task4/virtual-evict-q1.out
	./allocate -f cases/task4/virtual-evict-alt.txt -q 1 -m virtual | diff - cases/task4/virtual-evict-alt-q1.out
	./allocate -f cases/task4/to-evict.txt -q 3 -m virtual | diff - cases/task4/to-evict-q3.out

	./allocate -f cases/task1/spec.txt -q 1 -m infinite | diff - cases/task1/spec-q1.out
	./allocate -f cases/task3/simple-alloc.txt -q 3 -m paged | diff - cases/task3/simple-alloc-q3.out
	./allocate -f cases/task4/to-evict.txt -q 3 -m virtual | diff - cases/task4/to-evict-q3.out

	
clean: 
	rm -f allocate allocate.o list.o processManager.o