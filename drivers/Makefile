CC:=x86_64-os-gcc
CFLAGS+=-Wall -Wextra -Werror -O2 -MMD -Iinclude
PROGRAMS:=bin/atad bin/fbd bin/ps2d

all: $(PROGRAMS)
.SECONDEXPANSION:
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst bin/%d, %, $$@)/*.c))
	mkdir -p bin
	$(CC) -s -o $@ $^
install: all
	mkdir -p $(DESTDIR)/boot $(DESTDIR)/sbin
	cp bin/atad $(DESTDIR)/boot
	cp bin/fbd $(DESTDIR)/sbin
	cp bin/ps2d $(DESTDIR)/sbin
format:
	clang-format -i $(wildcard */*.c) $(wildcard include/*/*.h)
-include $(wildcard */*.d)
