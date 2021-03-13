CC:=x86_64-os-gcc
CFLAGS+=-Wall -Wextra -Werror -O2

CFILES:=main.c
OFILES:=$(patsubst %.c, %.o, $(CFILES))

all: bin/lelf
bin/lelf: $(OFILES)
	mkdir -p bin
	$(CC) -s -o bin/lelf $(OFILES) -Wl,-Ttext-segment,0x40000000
install: all
	mkdir -p $(DESTDIR)/boot
	cp bin/lelf $(DESTDIR)/boot
format:
	clang-format -i $(CFILES)
