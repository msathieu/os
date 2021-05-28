LLVM:=748c55d33cddb7cf09e202f5d7aac1c8042fe14a
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
$(LLVM).tar.gz:
	curl -OL https://github.com/msathieu/os-llvm/archive/$(LLVM).tar.gz
	echo "d25d38ab7d4b852b2186c191c720e9920bd286c54dafc516f368862c1d14134ce02e57717673e1befcc85a4760947296c79ba758797d861ff91e21fd5ac31781  $(LLVM).tar.gz" | shasum -c
build-toolchain: $(LLVM).tar.gz
	mkdir -p toolchain
	cd toolchain && tar -xf ../$(LLVM).tar.gz
	mkdir -p toolchain/os-llvm-$(LLVM)/build
	cd toolchain/os-llvm-$(LLVM)/build && CFLAGS= cmake ../llvm -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_DEFAULT_TARGET_TRIPLE="x86_64-os" -DDEFAULT_SYSROOT=$(DESTDIR) -DCMAKE_INSTALL_PREFIX=$(CURDIR)/toolchain
	cmake --build toolchain/os-llvm-$(LLVM)/build
	unset DESTDIR && cmake --install toolchain/os-llvm-$(LLVM)/build
	mkdir -p toolchain/lib/clang/12.0.0/share
	touch toolchain/lib/clang/12.0.0/share/cfi_blacklist.txt
clean:
	rm -rf toolchain/os-llvm-$(LLVM) *.tar.gz
.DELETE_ON_ERROR:
-include $(wildcard */*.d) monocypher.d
