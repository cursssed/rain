CC         = gcc
CFLAGS     = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
TEST_FLAGS = $(CFLAGS) -Wno-unused-parameter

.DEFAULT_GOAL := rain

rain: rain.c config.c config.h
	$(CC) $(CFLAGS) -o rain rain.c config.c -lncurses

test/rain_test: test/test_rain.c test/curses.h rain.c config.c config.h
	$(CC) $(TEST_FLAGS) -I test/ -o test/rain_test test/test_rain.c

test: test/rain_test
	./test/rain_test

clean:
	rm -f rain test/rain_test

.PHONY: rain test clean
