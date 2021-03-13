export DESTDIR:=$(CURDIR)/sysroot
export PATH:=$(PATH):$(CURDIR)/tools/toolchain/bin

build-grub: build
	-cp -n public.key loader-mb
	$(MAKE) install -Cloader-mb
	mkdir -p sysroot/boot/grub
	cp grub.cfg sysroot/boot/grub
	-test -f public.key && cp grub-signed.cfg sysroot/boot/grub/grub.cfg
	tools/bin/sign sign
	mkdir -p boot
	cp -r sysroot/boot boot
	grub-mkrescue -o os.iso boot -quiet
	mkdir -p system
	cp -r sysroot/bin system
	cp -r sysroot/lib system
	cp -r sysroot/sbin system
	tools/bin/svfs
	tools/bin/lvm
	dd if=lvm.img >> os.iso
	parted os.iso -a none mkpart primary `parted -m os.iso unit s print free | tail -n 1 | cut -f2 -d:` 100%
	sfdisk os.iso --part-type 2 0xb9 -q
build:
	-cp -n public.key kernel
	$(MAKE) install -Ckernel
	$(MAKE) all -Ctools
	$(MAKE) install-headers -Clibc
	$(MAKE) install -Clibc
	$(MAKE) install -Clibraries
	$(MAKE) install -Cinit
	$(MAKE) install -Cdrivers
	$(MAKE) install -Cservers
	-cp -n public.key filesystems
	$(MAKE) install -Cfilesystems
	$(MAKE) install -Cloadelf
	$(MAKE) install -Csh
	$(MAKE) install -Ccoreutils
run-grub: build-grub
	qemu-system-x86_64 -drive file=os.iso,format=raw -cpu max -serial stdio
run-grub-nographic: build-grub
	qemu-system-x86_64 -drive file=os.iso,format=raw -cpu max -nographic
toolchain:
	$(MAKE) install-headers -Clibc
	$(MAKE) build-toolchain -Ctools
format:
	clang-format -i $(shell find -name *.c) $(shell find -name *.h)
analyze: export CFLAGS:=-I$(DESTDIR)/usr/include
analyze:
	scan-build --status-bugs --use-cc=clang $(MAKE) install -Cloader-mb
	scan-build --status-bugs --use-cc=clang $(MAKE) install -Ckernel
	$(MAKE) install-headers -Clibc
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Clibc
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Clibraries
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Cinit
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Cdrivers
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Cservers
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Cfilesystems
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Cloadelf
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Csh
	scan-build --status-bugs --use-cc=x86_64-os-gcc $(MAKE) install -Ccoreutils
clean:
	rm -rf sysroot boot system *.img os.iso $(wildcard */bin) $(filter-out $(shell find ./tools/toolchain -name *.o), $(shell find -name *.o)) $(shell find -name *.d) $(wildcard */*.key) $(wildcard */*.bin) $(wildcard */*.a) libraries/lib
	$(MAKE) clean -Ctools
