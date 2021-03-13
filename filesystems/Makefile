PROGRAMS:=bin/lvmd bin/mbrd bin/svfsd bin/tmpd bin/vfsd

install: $(PROGRAMS)
	cp bin/lvmd $(DESTDIR)/boot
	cp bin/mbrd $(DESTDIR)/boot
	cp bin/svfsd $(DESTDIR)/boot
	cp bin/tmpd $(DESTDIR)/sbin
	cp bin/vfsd $(DESTDIR)/boot
.SECONDEXPANSION:
bin/svfsd: LDFLAGS:= -lmonocypher
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst bin/%d, %, $$@)/*.c)) $(patsubst %.key, %.o, $(wildcard public.key))
	mkdir -p bin
	$(CC) -s -o $@ $^ $(LDFLAGS)
%.o: %.key
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
-include $(wildcard */*.d)
