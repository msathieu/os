CFLAGS+=-ffreestanding

CFILES:=$(wildcard freestanding/*/*.c) $(wildcard */*.c)
SFILES:=$(wildcard freestanding/arch/x86_64/*.s) $(wildcard */*.s)
OFILES:=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.s, %.o, $(SFILES))

install: $(DESTDIR)/lib/libc.a crt0.o
	cp crt0.o $(DESTDIR)/lib
$(DESTDIR)/lib/libc.a: $(OFILES)
	$(AR) rcD $@ $(OFILES)
install-headers:
	mkdir -p $(DESTDIR)/include/__/freestanding
	cp -r include $(DESTDIR)
	cp freestanding/include/*.h $(DESTDIR)/include/__/freestanding
-include $(OFILES:.o=.d)
