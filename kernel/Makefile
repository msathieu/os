export ARCH=x86_64
CFLAGS+=--target=x86_64-elf -ffreestanding -fstack-protector-strong -mretpoline -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -flto -fvisibility=hidden -fsanitize=cfi -fbuiltin -Ilibc/include -Ilibelf -D__FLIBC_NO_FLOAT__
ASFLAGS+=--target=x86_64-elf

SFILES:=$(wildcard cpu/*.s)
CFILES:=main.c $(wildcard */*.c) $(wildcard sys/syscall/*.c)
OFILES:=$(CFILES:.c=.o) $(SFILES:.s=.o) public.o

$(DESTDIR)/boot/kernel.bin: $(OFILES) libc/libc.a
	$(CC) --target=x86_64-$(LDTARGET) -fuse-ld=lld -static -nostdlib -s -T link.ld -o $@ $^
libc/libc.a:
	$(MAKE) -Clibc
public.o: ../public.key
	$(OBJCOPY) -O elf64-x86-64 -I binary $^ $@
-include $(OFILES:.o=.d)
.DELETE_ON_ERROR:
