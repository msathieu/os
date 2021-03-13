PROGRAMS:=bin/argd bin/envd bin/ipcd bin/kbdd bin/logd bin/ttyd

install: $(PROGRAMS)
	cp bin/argd $(DESTDIR)/boot
	cp bin/envd $(DESTDIR)/sbin
	cp bin/ipcd $(DESTDIR)/boot
	cp bin/kbdd $(DESTDIR)/sbin
	cp bin/logd $(DESTDIR)/boot
	cp bin/ttyd $(DESTDIR)/sbin
.SECONDEXPANSION:
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst bin/%d, %, $$@)/*.c))
	mkdir -p bin
	$(CC) -s -o $@ $^
-include $(wildcard */*.d)
