CC:=x86_64-os-gcc
CFLAGS+=-Wall -Wextra -Werror -O2

CFILES:=main.c
OFILES:=$(patsubst %.c, %.o, $(CFILES))

all: sh
sh: $(OFILES)
	$(CC) -s -o sh $(OFILES)
install: all
	mkdir -p $(DESTDIR)/bin
	cp sh $(DESTDIR)/bin
format:
	clang-format -i $(CFILES)
clean:
	rm -f $(OFILES) sh
