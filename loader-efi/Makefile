export ARCH=x86_64
CFLAGS+=--target=x86_64-windows -ffreestanding -mretpoline -mno-red-zone -flto -fvisibility=hidden -fsanitize=cfi -fbuiltin -Ilibc/include
ASFLAGS+=--target=x86_64-windows
SFILES:=
CFILES:=main.c $(wildcard */*.c)
OFILES:=$(CFILES:.c=.o) $(SFILES:.s=.o)

$(DESTDIR)/boot/efi/boot/bootx64.efi: $(OFILES) libc/libc.a
	$(CC) --target=x86_64-windows -fuse-ld=lld -static -nostdlib -Wl,-entry:lmain -Wl,-subsystem:efi_application -o $@ $^
libc/libc.a:
	$(MAKE) -Clibc
-include $(OFILES:.o=.d)
.DELETE_ON_ERROR:
