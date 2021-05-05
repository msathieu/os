LIBRARIES:=$(DESTDIR)/lib/libmonocypher.a

install: $(LIBRARIES)
	cp -r include $(DESTDIR)
.SECONDEXPANSION:
$(LIBRARIES): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst $(DESTDIR)/lib/lib%.a, %, $$@)/*.c))
	$(AR) rcD $@ $^
-include $(wildcard */*.d)
