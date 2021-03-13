BINUTILS:=f74f7dc79e96079f2bc55bfe5d6d4e0edeffdd55
GCC:=f30188d8a08a03302d987160f74e701d44234055
PROGRAMS:=bin/lvm bin/sign bin/svfs
export CC:=cc
export AR:=ar
export AS:=as
export OBJCOPY:=objcopy

all: $(PROGRAMS)
.SECONDEXPANSION:
$(PROGRAMS): $$(patsubst %.c, %.o, $$(wildcard $$(patsubst bin/%, %, $$@)/*.c)) monocypher.o
	mkdir -p bin
	$(CC) -s -o $@ $^
$(BINUTILS).tar.gz:
	wget https://github.com/MSathieu/os-binutils/archive/$(BINUTILS).tar.gz
	echo "c3dff1886a3463bd7e5c0e986f692436eeabd7bf10481ada035cfbc795ebb83a9826d1fc1819cc04784a779066bf7166f6776c574e122c55fdbfee51c8414c4d $(BINUTILS).tar.gz" | sha512sum -c
$(GCC).tar.gz:
	wget https://github.com/MSathieu/os-gcc/archive/$(GCC).tar.gz
	echo "fb5c07f31026277cb0990f520af3aa82467b8a95002e652594f6df45a31bb5a9b2accc7be4a933b2962361f2f6894c67a9f01cb07ec3feb2fc5eb1ca9efdbec3 $(GCC).tar.gz" | sha512sum -c
build-toolchain: $(BINUTILS).tar.gz $(GCC).tar.gz
	mkdir -p toolchain
	cd toolchain && tar -xf ../$(BINUTILS).tar.gz
	cd toolchain && tar -xf ../$(GCC).tar.gz
	mkdir -p toolchain/os-binutils-$(BINUTILS)/build
	cd toolchain/os-binutils-$(BINUTILS)/build && CFLAGS= ../configure --target=x86_64-os --with-sysroot=$(DESTDIR) --prefix=$(CURDIR)/toolchain
	cd toolchain/os-binutils-$(BINUTILS)/build && $(MAKE)
	unset DESTDIR && cd toolchain/os-binutils-$(BINUTILS)/build && $(MAKE) install-strip
	mkdir -p toolchain/os-gcc-$(GCC)/build
	cd toolchain/os-gcc-$(GCC)/build && CFLAGS= ../configure --target=x86_64-os --with-sysroot=$(DESTDIR) --enable-languages=c --prefix=$(CURDIR)/toolchain
	cd toolchain/os-gcc-$(GCC)/build && $(MAKE) all-gcc all-target-libgcc
	unset DESTDIR && cd toolchain/os-gcc-$(GCC)/build && $(MAKE) install-strip-gcc install-strip-target-libgcc
clean:
	rm -rf toolchain/os-binutils-$(BINUTILS) toolchain/os-gcc-$(GCC) *.tar.gz
.DELETE_ON_ERROR:
-include $(wildcard */*.d) monocypher.d
