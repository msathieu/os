export DESTDIR:=$(CURDIR)/sysroot
export PATH:=$(CURDIR)/tools/toolchain/bin:$(PATH)
export CC:=clang
export AR:=llvm-ar
export AS:=clang
export OBJCOPY:=llvm-objcopy
export CFLAGS:=-Wall -Wextra -Werror -O2 -MD -Iinclude -Wshadow
export ASFLAGS:=-c
export LDTARGET:=elf
ifeq ($(shell uname),Darwin)
	export LDTARGET:=linux-elf
endif
ifdef UBSAN
	CFLAGS+=-fsanitize=undefined
endif

build-grub: build
	$(MAKE) -Cloader-mb
	mkdir -p sysroot/boot/boot/grub
	cp grub.cfg sysroot/boot/boot/grub
	grub-mkrescue -o os.iso sysroot/boot -quiet
	cat lvm.img >> os.iso
	tools/bin/mbr
build-efi: build
	mkdir -p sysroot/boot/efi/boot
	$(MAKE) -Cloader-efi
	dd if=/dev/zero of=boot.img bs=1k count=2880
	mformat -i boot.img
	mcopy -i boot.img sysroot/boot/* ::
	mmd -i boot.img efi/boot
	mcopy -i boot.img sysroot/boot/efi/boot/bootx64.efi ::/efi/boot
	mkdir -p iso
	cp boot.img iso
	xorriso -as mkisofs -e boot.img --protective-msdos-label -o os.iso iso
	cat lvm.img >> os.iso
	tools/bin/mbr
private.key:
	CFLAGS="$(CFLAGS) -fno-sanitize=all" $(MAKE) -Ctools
	tools/bin/sign generate
build: private.key
	CFLAGS="$(CFLAGS) -fno-sanitize=all" $(MAKE) -Ctools
	mkdir -p sysroot/boot sysroot/sbin sysroot/bin sysroot/lib sysroot/include system
	$(MAKE) -Ckernel
	$(MAKE) install-headers -Clibc
	$(MAKE) -Clibc
	$(MAKE) -Clibraries
	$(MAKE) -Cinit
	$(MAKE) -Cdrivers
	$(MAKE) -Cacpid
	$(MAKE) -Cservers
	$(MAKE) -Cfilesystems
	$(MAKE) -Cloadelf
	$(MAKE) -Csh
	$(MAKE) -Ccoreutils
	tools/bin/sign sign
	cp -r sysroot/bin system
	cp -r sysroot/include system
	cp -r sysroot/lib system
	cp -r sysroot/sbin system
	tools/bin/svfs
	tools/bin/lvm
QEMU:=qemu-system-x86_64 -drive file=os.iso,format=raw -cpu max -m 512 -smp 2
run-efi: build-efi
	$(QEMU) -serial stdio -bios OVMF.fd
run-grub: build-grub
	$(QEMU) -serial stdio
run-grub-nographic: build-grub
	$(QEMU) -nographic
toolchain:
	$(MAKE) install-headers -Clibc
	$(MAKE) build-toolchain -Ctools
format:
	clang-format -i $(filter-out $(shell find ./acpid/lai -name *.c) $(shell find ./acpid/lai -name *.h), $(shell find . -name *.c) $(shell find . -name *.h))
analyze:
	CFLAGS="$(CFLAGS) -fno-sanitize=all" $(MAKE) -Ctools
	scan-build --status-bugs --use-cc=clang $(MAKE) build-grub build-efi
clean:
	$(MAKE) clean -Ctools
	rm -rf iso sysroot system *.img os.iso tools/bin `find . -name *.o` `find . -name *.d` loader-mb/libc/libc.a loader-efi/libc/libc.a kernel/libc/libc.a
