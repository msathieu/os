CFLAGS+=-ffreestanding
CFILES:=$(wildcard freestanding/*/*.c) $(wildcard */*.c) $(wildcard freestanding/arch/x86_64/*.c)
SFILES:=$(wildcard freestanding/arch/x86_64/*.s) $(wildcard */*.s)
OFILES:=$(CFILES:.c=.o) $(SFILES:.s=.o)

install: $(DESTDIR)/lib/libc.a crt0.o
	cp crt0.o $(DESTDIR)/lib
$(DESTDIR)/lib/libc.a: $(OFILES)
	$(AR) rcD $@ $(OFILES)
	$(AR) d $@ calloc.o
install-headers:
	mkdir -p $(DESTDIR)/include/__/freestanding
	cp -rp include $(DESTDIR)
	cp -rp freestanding/include/* $(DESTDIR)/include/__/freestanding
-include $(OFILES:.o=.d)
.DELETE_ON_ERROR:
