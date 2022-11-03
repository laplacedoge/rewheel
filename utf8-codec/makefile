CC = @gcc
RM = @rm -rf

.PHONY: all test clean

all:
	@echo "NOTHING TO DO"

utf8_codec.o: utf8_codec.c utf8_codec.h
	$(CC) -c -o utf8_codec.o utf8_codec.c

test.o: test.c utf8_codec.h
	$(CC) -c -o test.o test.c

test: test.o utf8_codec.o
	$(CC) -o test test.o utf8_codec.o
	@./test

clean:
	$(RM) *.o main test
