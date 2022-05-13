CFLAGS+=-Ilibelf
$(DESTDIR)/boot/lelf: main.o libelf/libelf.a
	$(CC) -s -o $@ $^ -Wl,--image-base,0x40000000
libelf/libelf.a:
	$(MAKE) -Clibelf
.DELETE_ON_ERROR:
.PHONY: libelf/libelf.a
