PROGRAMS:=$(DESTDIR)/boot/argd $(DESTDIR)/sbin/envd $(DESTDIR)/boot/ipcd $(DESTDIR)/sbin/kbdd $(DESTDIR)/boot/logd $(DESTDIR)/sbin/ttyd

all: $(PROGRAMS)
.SECONDEXPANSION:
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(notdir $$@)/*.c))
	$(CC) -s -o $@ $^
-include $(wildcard */*.d)
.DELETE_ON_ERROR:
