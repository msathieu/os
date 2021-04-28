CFLAGS+=--target=x86_64-elf -ffreestanding --sysroot=/dev/null -fstack-protector-strong -mretpoline -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -flto -fvisibility=hidden -fsanitize=cfi -fbuiltin -Ilibc/include
ASFLAGS+=--target=x86_64-elf

LIBC:=$(patsubst %.c, %.o, $(wildcard libc/*/*.c)) $(patsubst %.s, %.o, $(wildcard libc/arch/x86_64/*.s))
SFILES:=$(wildcard cpu/*.s)
CFILES:=main.c $(wildcard */*.c) $(wildcard sys/syscall/*.c)
PUBKEY:=$(wildcard public.key)
OFILES:=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.s, %.o, $(SFILES)) $(patsubst %.key, %.o, $(PUBKEY))

$(LIBC): CFLAGS+=-fno-builtin
$(DESTDIR)/boot/kernel.bin: $(OFILES) $(LIBC)
	$(CC) --target=x86_64-$(LDTARGET) -fuse-ld=lld -static -nostdlib -s -T link.ld -o $@ $^
%.o: %.key
	$(OBJCOPY) -O elf64-x86-64 -I binary $^ $@
-include $(OFILES:.o=.d)
