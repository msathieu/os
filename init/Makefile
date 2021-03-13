CC:=x86_64-os-gcc
CFLAGS+=-Wall -Wextra -Werror -O2

CFILES:=main.c
OFILES:=$(patsubst %.c, %.o, $(CFILES))

all: bin/init
bin/init: $(OFILES)
	mkdir -p bin
	$(CC) -s -o bin/init $(OFILES)
install: all
	mkdir -p $(DESTDIR)/boot
	cp bin/init $(DESTDIR)/boot
format:
	clang-format -i $(CFILES)
