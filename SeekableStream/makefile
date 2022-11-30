CC = @gcc
RM = @rm -rf

.PHONY: all test clean

all:
	@echo "NOTHING TO DO"

SeekableStream.o: SeekableStream.c SeekableStream.h
	$(CC) -c -o SeekableStream.o SeekableStream.c

test.o: test.c SeekableStream.h
	$(CC) -c -o test.o test.c

test: test.o SeekableStream.o
	$(CC) -o test test.o SeekableStream.o
	@./test

clean:
	$(RM) *.o main test
