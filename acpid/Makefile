CFLAGS+=-Ilai/include -Wno-shadow
LAI:=$(patsubst %.c, %.o, $(wildcard lai/*/*.c))

$(DESTDIR)/boot/acpid: $(LAI) $(patsubst %.c, %.o, $(wildcard *.c))
	$(CC) -s -o $@ $^
-include $(wildcard */*.d)
.DELETE_ON_ERROR:
