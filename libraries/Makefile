LIBRARIES:=$(DESTDIR)/lib/libmonocypher.a

install: $(LIBRARIES)
	cp -r include $(DESTDIR)/usr
.SECONDEXPANSION:
$(LIBRARIES): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst $(DESTDIR)/lib/lib%.a, %, $$@)/*.c))
	$(AR) rc $@ $^
-include $(wildcard */*.d)
