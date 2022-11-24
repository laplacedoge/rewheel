CC = @gcc
RM = @rm -rf

.PHONY: all test clean

all:
	@echo "NOTHING TO DO"

stream.o: stream.c stream.h
	$(CC) -c -o stream.o stream.c

test.o: test.c stream.h
	$(CC) -c -o test.o test.c

test: test.o stream.o
	$(CC) -o test test.o stream.o
	@./test

clean:
	$(RM) *.o main test
