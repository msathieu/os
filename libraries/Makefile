CC:=x86_64-os-gcc
AR:=x86_64-os-ar
CFLAGS+=-Wall -Wextra -Werror -O2 -MMD -Iinclude
LIBRARIES:=lib/libmonocypher.a

all: $(LIBRARIES)
.SECONDEXPANSION:
$(LIBRARIES): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst lib/lib%.a, %, $$@)/*.c))
	mkdir -p lib
	$(AR) rc $@ $^
install: all
	mkdir -p $(DESTDIR)/usr
	cp -r include $(DESTDIR)/usr
	cp -r lib $(DESTDIR)
format:
	clang-format -i $(wildcard */*.c) $(wildcard include/*.h)
-include $(wildcard */*.d)
