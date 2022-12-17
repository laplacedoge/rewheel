CC = @gcc
RM = @rm -rf

.PHONY: all test clean

all:
	@echo "NOTHING TO DO"

kv.o: kv.c kv.h
	$(CC) -c -o kv.o kv.c

test.o: test.c kv.h
	$(CC) -c -o test.o test.c

test: test.o kv.o
	$(CC) -o test test.o kv.o
	@./test

clean:
	$(RM) *.o main test
