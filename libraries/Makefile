LIBRARIES:=$(DESTDIR)/lib/libmonocypher.a $(DESTDIR)/lib/libelf.a

install: $(LIBRARIES)
	cp -rp include $(DESTDIR)
	cp -p elf/elf.h $(DESTDIR)/include
.SECONDEXPANSION:
$(LIBRARIES): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst $(DESTDIR)/lib/lib%.a, %, $$@)/*.c))
	$(AR) rcD $@ $^
-include $(wildcard */*.d)
.DELETE_ON_ERROR:
