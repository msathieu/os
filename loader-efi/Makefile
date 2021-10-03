CFLAGS+=--target=x86_64-windows -ffreestanding -fstack-protector-strong -mretpoline -mno-red-zone -flto -fvisibility=hidden -fsanitize=cfi -fbuiltin -Ilibc/include
ASFLAGS+=--target=x86_64-windows

LIBC:=$(patsubst %.c, %.o, $(filter-out $(wildcard libc/math/*.c), $(wildcard libc/*/*.c))) $(patsubst %.s, %.o, $(wildcard libc/arch/x86_64/*.s))
SFILES:=
CFILES:=main.c $(wildcard */*.c)
OFILES:=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.s, %.o, $(SFILES))

$(LIBC): CFLAGS+=-fno-builtin
$(DESTDIR)/boot/efi/boot/bootx64.efi: $(OFILES) libc.a
	$(CC) --target=x86_64-windows -fuse-ld=lld -static -nostdlib -Wl,-entry:lmain -Wl,-subsystem:efi_application -o $@ $^
libc.a: $(LIBC)
	$(AR) rcD $@ $(LIBC)
-include $(OFILES:.o=.d)
.DELETE_ON_ERROR:
