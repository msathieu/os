CC:=x86_64-os-gcc
CFLAGS+=-Wall -Wextra -Werror -O2

CFILES:=main.c
OFILES:=$(patsubst %.c, %.o, $(CFILES))

all: lelf
lelf: $(OFILES)
	$(CC) -s -o lelf $(OFILES) -Wl,-Ttext-segment,0x40000000
install: all
	mkdir -p $(DESTDIR)/boot
	cp lelf $(DESTDIR)/boot
format:
	clang-format -i $(CFILES)
clean:
	rm -f $(OFILES) lelf
