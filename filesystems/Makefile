CC:=x86_64-os-gcc
OBJCOPY:=x86_64-os-objcopy
CFLAGS+=-Wall -Wextra -Werror -O2 -MMD -Iinclude
PROGRAMS:=bin/lvmd bin/mbrd bin/svfsd bin/tmpd bin/vfsd

all: $(PROGRAMS)
.SECONDEXPANSION:
bin/svfsd: LDFLAGS:= -lmonocypher
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst bin/%d, %, $$@)/*.c)) $(patsubst %.key, %.o, $(wildcard public.key))
	mkdir -p bin
	$(CC) -s -o $@ $^ $(LDFLAGS)
%.o: %.key
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
install: all
	mkdir -p $(DESTDIR)/boot $(DESTDIR)/sbin
	cp bin/lvmd $(DESTDIR)/boot
	cp bin/mbrd $(DESTDIR)/boot
	cp bin/svfsd $(DESTDIR)/boot
	cp bin/tmpd $(DESTDIR)/sbin
	cp bin/vfsd $(DESTDIR)/boot
format:
	clang-format -i $(wildcard */*.c) $(wildcard include/*/*.h)
clean:
	rm -rf public.key bin $(wildcard */*.o) $(wildcard */*.d)
-include $(wildcard */*.d)
