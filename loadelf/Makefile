$(DESTDIR)/boot/lelf: main.o
	$(CC) -s -o $@ $^ -Wl,--image-base,0x40000000
