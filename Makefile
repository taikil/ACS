.phony all:
all: ACS

ACS: ACS.c customers.c
	gcc -Wall -pthread ACS.c customers.c  -o ACS -g

.PHONY clean:
clean:
	-rm -rf *.o *.exe
