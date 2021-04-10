BINUTILS:=572df6d4e00649f0e69baa11008e1be357eb51ab
GCC:=1db3f95fed7a95c0338ce0c33860229ddc7172a6
PROGRAMS:=bin/lvm bin/mbr bin/sign bin/svfs
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
	echo "114b99e26d8f5e21d806bce1a5371d1454a278441a8d59cc337452cfd1e376941592a1a313be60e351fed6ba1d0ef145c74cd964453ecab5e8064366cd9d2f79  $(BINUTILS).tar.gz" | shasum -c
$(GCC).tar.gz:
	wget https://github.com/MSathieu/os-gcc/archive/$(GCC).tar.gz
	echo "b601858e876c8904a17a40bf8fbc12dadf1bdfff18ceaf492c23c0e1257e95f0236af4d6e33e144ca09a45b157f34e2af57d261c1bb77ac3be74e6cf288f06b9  $(GCC).tar.gz" | shasum  -c
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
