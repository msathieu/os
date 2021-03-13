PROGRAMS:=$(DESTDIR)/boot/lvmd $(DESTDIR)/boot/mbrd $(DESTDIR)/boot/svfsd $(DESTDIR)/sbin/tmpd $(DESTDIR)/boot/vfsd

all: $(PROGRAMS)
.SECONDEXPANSION:
$(DESTDIR)/boot/svfsd: LDFLAGS:= -lmonocypher
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(notdir $$@)/*.c)) $(patsubst %.key, %.o, $(wildcard public.key))
	$(CC) -s -o $@ $^ $(LDFLAGS)
%.o: %.key
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
-include $(wildcard */*.d)
