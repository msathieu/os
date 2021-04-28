LLVM:=59b1cc0de9315a316417daddb2afa740f4fedb5f
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
	wget https://github.com/msathieu/os-llvm/archive/$(LLVM).tar.gz
	echo "f70d44fc7e62f65a4c9cbc323b0dcb6e2ec3b874dadc6d835139b6b949ac03ae6064e9191e3c28eb66cabec86519ec640970eab1e323c483a6be1cb0a19df02a  $(LLVM).tar.gz" | shasum -c
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
