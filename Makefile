export DESTDIR:=$(CURDIR)/sysroot
export PATH:=$(CURDIR)/tools/toolchain/bin:$(PATH)
export CC:=clang
export AR:=llvm-ar
export AS:=clang
export OBJCOPY:=llvm-objcopy
export CFLAGS:=-Wall -Wextra -Werror -O2 -MMD -Iinclude -Wshadow
export ASFLAGS:=-c
export LDTARGET:=elf
ifeq ($(shell uname),Darwin)
export LDTARGET:=linux-elf
endif
ifdef UBSAN
CFLAGS+=-fsanitize=undefined
endif

build-grub: build-multiboot
	mkdir -p sysroot/boot/grub
	cp grub.cfg sysroot/boot/grub
	tools/bin/sign sign
	mkdir -p boot
	cp -r sysroot/boot boot
	grub-mkrescue -o os.iso boot -quiet
	mkdir -p system
	cp -r sysroot/bin system
	cp -r sysroot/include system
	cp -r sysroot/lib system
	cp -r sysroot/sbin system
	tools/bin/svfs
	tools/bin/lvm
	cat lvm.img >> os.iso
	tools/bin/mbr
build-multiboot: build
	$(MAKE) -Cloader-mb
private.key:
	CFLAGS="$(CFLAGS) -fno-sanitize=all" $(MAKE) -Ctools
	tools/bin/sign generate
build: private.key
	CFLAGS="$(CFLAGS) -fno-sanitize=all" $(MAKE) -Ctools
	mkdir -p sysroot/boot sysroot/sbin sysroot/bin sysroot/lib sysroot/include
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
run-grub: build-grub
	qemu-system-x86_64 -drive file=os.iso,format=raw -cpu max -serial stdio
run-grub-nographic: build-grub
	qemu-system-x86_64 -drive file=os.iso,format=raw -cpu max -nographic
toolchain:
	$(MAKE) install-headers -Clibc
	$(MAKE) build-toolchain -Ctools
format:
	clang-format -i $(filter-out $(shell find ./acpid/lai -name *.c) $(shell find ./acpid/lai -name *.h), $(shell find . -name *.c) $(shell find . -name *.h))
analyze:
	CFLAGS="$(CFLAGS) -fno-sanitize=all" $(MAKE) -Ctools
	scan-build --status-bugs --use-cc=clang $(MAKE) build-multiboot
clean:
	$(MAKE) clean -Ctools
	rm -rf sysroot boot system *.img os.iso tools/bin `find . -name *.o` `find . -name *.d`
