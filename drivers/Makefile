PROGRAMS:=bin/atad bin/fbd bin/ps2d

install: $(PROGRAMS)
	cp bin/atad $(DESTDIR)/boot
	cp bin/fbd $(DESTDIR)/sbin
	cp bin/ps2d $(DESTDIR)/sbin
.SECONDEXPANSION:
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst bin/%d, %, $$@)/*.c))
	mkdir -p bin
	$(CC) -s -o $@ $^
-include $(wildcard */*.d)
