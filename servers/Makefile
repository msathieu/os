CC:=x86_64-os-gcc
CFLAGS+=-Wall -Wextra -Werror -O2 -MMD -Iinclude
PROGRAMS:=bin/argd bin/envd bin/ipcd bin/kbdd bin/logd bin/ttyd

all: $(PROGRAMS)
.SECONDEXPANSION:
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst bin/%d, %, $$@)/*.c))
	mkdir -p bin
	$(CC) -s -o $@ $^
install: all
	mkdir -p $(DESTDIR)/boot $(DESTDIR)/sbin
	cp bin/argd $(DESTDIR)/boot
	cp bin/envd $(DESTDIR)/sbin
	cp bin/ipcd $(DESTDIR)/boot
	cp bin/kbdd $(DESTDIR)/sbin
	cp bin/logd $(DESTDIR)/boot
	cp bin/ttyd $(DESTDIR)/sbin
format:
	clang-format -i $(wildcard */*.c) $(wildcard include/*/*.h)
-include $(wildcard */*.d)
