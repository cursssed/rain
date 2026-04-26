CC          = gcc
VERSION     ?= 0.2.0
CFLAGS      = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -O2 -DRAIN_VERSION=\"$(VERSION)\"
LDLIBS      = -lncurses
TEST_FLAGS  = $(CFLAGS) -Wno-unused-parameter
BENCH_FLAGS = $(CFLAGS) -Wno-unused-parameter

PREFIX    ?= /usr/local
BINDIR    ?= $(PREFIX)/bin
DOCDIR    ?= $(PREFIX)/share/doc/rain

.DEFAULT_GOAL := rain

config_template.h: rain.conf.example gen_template.sh
	./gen_template.sh rain.conf.example > $@

rain: rain.c config.c config.h config_template.h
	$(CC) $(CFLAGS) -o rain rain.c config.c $(LDLIBS)

test/rain_test: test/test_rain.c test/curses.h rain.c config.c config.h config_template.h
	$(CC) $(TEST_FLAGS) -I test/ -I . -o test/rain_test test/test_rain.c

test: test/rain_test
	./test/rain_test

bench/bench_frame: bench/bench_frame.c bench/curses.h rain.c config.c config.h config_template.h
	$(CC) $(BENCH_FLAGS) -I bench/ -I . -o bench/bench_frame bench/bench_frame.c

bench: bench/bench_frame
	./bench/bench_frame

install: rain
	install -Dm755 rain $(DESTDIR)$(BINDIR)/rain
	install -Dm644 rain.conf.example $(DESTDIR)$(DOCDIR)/rain.conf.example
	install -Dm644 LICENSE $(DESTDIR)$(PREFIX)/share/licenses/rain/LICENSE

clean:
	rm -f rain test/rain_test bench/bench_frame config_template.h

.PHONY: rain test bench clean install
