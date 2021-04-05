PROGRAMS:=$(DESTDIR)/sbin/devd $(DESTDIR)/boot/lvmd $(DESTDIR)/boot/mbrd $(DESTDIR)/boot/svfsd $(DESTDIR)/sbin/tmpd $(DESTDIR)/boot/vfsd
DEVICES:=$(patsubst dev/%.c, $(DESTDIR)/sbin/dev-%, $(wildcard dev/*.c))

all: $(PROGRAMS) $(DEVICES)
.SECONDEXPANSION:
$(DESTDIR)/boot/svfsd: LDFLAGS:=-lmonocypher
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(notdir $$@)/*.c)) $(patsubst %.key, %.o, $(wildcard public.key))
	$(CC) -s -o $@ $^ $(LDFLAGS)
$(DEVICES): $(DESTDIR)/sbin/dev-%: dev/%.o
	$(CC) -s -o $@ $^
%.o: %.key
	$(OBJCOPY) -O elf64-x86-64 -I binary $^ $@
-include $(wildcard */*.d)
