CC:=clang
AS:=clang
AR:=llvm-ar
OBJCOPY:=llvm-objcopy
CFLAGS:=--target=x86_64-elf -ffreestanding --sysroot=/dev/null -fstack-protector-strong -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -Werror -fbuiltin -O2 -MMD -Iinclude -Ilibc/include
ASFLAGS:=--target=x86_64-elf -c

LIBC:=$(patsubst %.c, %.o, $(wildcard libc/*/*.c))
SFILES:=$(wildcard cpu/*.s)
CFILES:=main.c $(wildcard */*.c) $(wildcard sys/syscall/*.c)
PUBKEY:=$(wildcard public.key)
OFILES:=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.s, %.o, $(SFILES)) $(patsubst %.key, %.o, $(PUBKEY))

all: kernel.bin
kernel.bin: libc.a $(OFILES)
	$(CC) --target=x86_64-elf -static -nostdlib -s -T link.ld -o kernel.bin $(OFILES) libc.a
%.o: %.key
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
$(LIBC): CFLAGS+=-fno-builtin
libc.a: $(LIBC)
	$(AR) --format=gnu rc libc.a $(LIBC)
install: all
	mkdir -p $(DESTDIR)/boot
	cp kernel.bin $(DESTDIR)/boot
-include $(OFILES:.o=.d)
