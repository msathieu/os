CFLAGS+=--target=i686-elf -ffreestanding --sysroot=/dev/null -fstack-protector-strong -mretpoline -mno-mmx -mno-sse -flto -fvisibility=hidden -fsanitize=cfi -fbuiltin -Ilibc/include
ASFLAGS+=--target=i686-elf

LIBC:=$(patsubst %.c, %.o, $(wildcard libc/*/*.c) $(wildcard libc/arch/generic/*.c))
SFILES:=$(wildcard cpu/*.s)
CFILES:=main.c $(wildcard */*.c)
PUBKEY:=$(wildcard public.key)
OFILES:=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.s, %.o, $(SFILES)) $(patsubst %.key, %.o, $(PUBKEY))

$(LIBC): CFLAGS+=-fno-builtin
$(DESTDIR)/boot/loader.bin: $(OFILES) $(LIBC)
	$(CC) --target=i686-$(LDTARGET) -fuse-ld=lld -static -nostdlib -s -T link.ld -o $@ $^
%.o: %.key
	$(OBJCOPY) -O elf32-i386 -I binary $^ $@
-include $(OFILES:.o=.d)
