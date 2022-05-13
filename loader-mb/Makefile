export ARCH=i686
CFLAGS+=--target=i686-elf -ffreestanding -fstack-protector-strong -mretpoline -mno-mmx -mno-sse -flto -fvisibility=hidden -fsanitize=cfi -fbuiltin -Ilibc/include -Ilibelf -D__FLIBC_NO_FLOAT__
ASFLAGS+=--target=i686-elf

SFILES:=$(wildcard cpu/*.s)
CFILES:=main.c $(wildcard */*.c)
OFILES:=$(CFILES:.c=.o) $(SFILES:.s=.o) public.o

$(DESTDIR)/boot/loader.bin: $(OFILES) libc/libc.a
	$(CC) --target=i686-$(LDTARGET) -fuse-ld=lld -static -nostdlib -s -T link.ld -o $@ $^
libc/libc.a:
	$(MAKE) -Clibc
public.o: ../public.key
	$(OBJCOPY) -O elf32-i386 -I binary $^ $@
-include $(OFILES:.o=.d)
.DELETE_ON_ERROR:
