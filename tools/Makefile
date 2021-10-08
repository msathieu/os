LLVM:=5bd6fc1a340cb257a87a239a1681242c5495911e
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
	echo "aa14aec75de9afaa89a58aa93141881e0b2c2aa58dbfe023bcbb9999d4ba3673290b130328212da39697ab84f5ed655c2ed9d1047ec74c2094aa46ef7a4f3482  $(LLVM).tar.gz" | shasum -c
build-toolchain: $(LLVM).tar.gz
	mkdir -p toolchain
	cd toolchain && tar -xf ../$(LLVM).tar.gz
	mkdir -p toolchain/os-llvm-$(LLVM)/build
	cd toolchain/os-llvm-$(LLVM)/build && CFLAGS= cmake ../llvm -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_DEFAULT_TARGET_TRIPLE="x86_64-os" -DDEFAULT_SYSROOT=$(DESTDIR) -DCMAKE_INSTALL_PREFIX=$(CURDIR)/toolchain
	cmake --build toolchain/os-llvm-$(LLVM)/build
	unset DESTDIR && cmake --install toolchain/os-llvm-$(LLVM)/build
	mkdir -p toolchain/lib/clang/13.0.0/share
	touch toolchain/lib/clang/13.0.0/share/cfi_ignorelist.txt
clean:
	rm -rf toolchain/os-llvm-* *.tar.gz
-include $(wildcard */*.d) monocypher.d
.DELETE_ON_ERROR:
