PROGRAMS:=$(DESTDIR)/boot/atad $(DESTDIR)/sbin/fbd $(DESTDIR)/boot/pcid $(DESTDIR)/sbin/ps2d

all: $(PROGRAMS)
.SECONDEXPANSION:
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(notdir $$@)/*.c))
	$(CC) -s -o $@ $^
-include $(wildcard */*.d)
