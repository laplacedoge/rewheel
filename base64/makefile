CC = @gcc
RM = @rm -rf

.PHONY: all test clean

all:
	@echo "NOTHING TO DO"

base64.o: base64.c base64.h
	$(CC) -c -o base64.o base64.c

test.o: test.c base64.h
	$(CC) -c -o test.o test.c

test: test.o base64.o
	$(CC) -o test test.o base64.o
	@./test

clean:
	$(RM) *.o main test
