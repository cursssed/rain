CC         = gcc
CFLAGS     = -Wall -Wextra -std=c11
TEST_FLAGS = $(CFLAGS) -D_POSIX_C_SOURCE=200809L -Wno-unused-parameter -Wno-return-type

.DEFAULT_GOAL := rain

rain: rain.c
	$(CC) $(CFLAGS) -o rain rain.c -lncurses

test/rain_test: test/test_rain.c test/curses.h
	$(CC) $(TEST_FLAGS) -I test/ -o test/rain_test test/test_rain.c

test: test/rain_test
	./test/rain_test

clean:
	rm -f rain test/rain_test

.PHONY: rain test clean
