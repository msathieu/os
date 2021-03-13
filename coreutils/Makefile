CC:=x86_64-os-gcc
CFLAGS+=-Wall -Wextra -Werror -O2
PROGRAMS:=$(patsubst %.c, bin/%, $(wildcard *.c))

all: $(PROGRAMS)
$(PROGRAMS): bin/%: %.o
	mkdir -p bin
	$(CC) -s -o $@ $<
install: all
	mkdir -p $(DESTDIR)
	cp -r bin $(DESTDIR)
format:
	clang-format -i $(wildcard *.c)
clean:
	rm -rf bin $(wildcard *.o)
