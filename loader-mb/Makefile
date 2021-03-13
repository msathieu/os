CC:=clang
AS:=clang
AR:=llvm-ar
OBJCOPY:=llvm-objcopy
CFLAGS:=--target=i686-elf -ffreestanding --sysroot=/dev/null -fstack-protector-strong -Wall -Wextra -Werror -fbuiltin -O2 -MMD -Iinclude -Ilibc/include
ASFLAGS:=--target=i686-elf -c

LIBC:=$(patsubst %.c, %.o, $(wildcard libc/*/*.c))
SFILES:=$(wildcard cpu/*.s)
CFILES:=main.c $(wildcard */*.c)
PUBKEY:=$(wildcard public.key)
OFILES:=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.s, %.o, $(SFILES)) $(patsubst %.key, %.o, $(PUBKEY))

all: loader.bin
loader.bin: libc.a $(OFILES)
	$(CC) --target=i686-elf -static -nostdlib -s -T link.ld -o loader.bin $(OFILES) libc.a
%.o: %.key
	$(OBJCOPY) -O elf32-i386 -I binary $< $@
$(LIBC): CFLAGS+=-fno-builtin
libc.a: $(LIBC)
	$(AR) --format=gnu rc libc.a $(LIBC)
install: all
	mkdir -p $(DESTDIR)/boot
	cp loader.bin $(DESTDIR)/boot
format:
	clang-format -i $(CFILES) $(wildcard include/*.h) $(wildcard include/*/*.h)
	$(MAKE) format -Clibc
clean:
	rm -f public.key $(OFILES) $(OFILES:.o=.d) $(LIBC) $(LIBC:.o=.d) libc.a loader.bin
-include $(OFILES:.o=.d)
