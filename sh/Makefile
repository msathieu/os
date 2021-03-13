CC:=x86_64-os-gcc
CFLAGS+=-Wall -Wextra -Werror -O2

CFILES:=main.c
OFILES:=$(patsubst %.c, %.o, $(CFILES))

all: bin/sh
bin/sh: $(OFILES)
	mkdir -p bin
	$(CC) -s -o bin/sh $(OFILES)
install: all
	mkdir -p $(DESTDIR)/bin
	cp bin/sh $(DESTDIR)/bin
format:
	clang-format -i $(CFILES)
