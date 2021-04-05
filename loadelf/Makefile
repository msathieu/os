$(DESTDIR)/boot/lelf: main.o
	$(CC) -s -o $@ $^ -Wl,-Ttext-segment,0x40000000
