LLVM:=365de983edbcc044fcef36c18c8af8fac0f9704a
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
	wget https://github.com/MSathieu/os-llvm/archive/$(LLVM).tar.gz
	echo "5b7eef3eff013054f3381dac6bb34cf1bc5a70c3d722f8adf0975ae88c1a1df616780d544713ca2c8e15d752906f924f1186ab7d052b41a621d2baf90ee5e799  $(LLVM).tar.gz" | shasum -c
build-toolchain: $(LLVM).tar.gz
	mkdir -p toolchain
	cd toolchain && tar -xf ../$(LLVM).tar.gz
	mkdir -p toolchain/os-llvm-$(LLVM)/build
	cd toolchain/os-llvm-$(LLVM)/build && CFLAGS= cmake ../llvm -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_DEFAULT_TARGET_TRIPLE="x86_64-os" -DDEFAULT_SYSROOT=$(DESTDIR) -DCMAKE_INSTALL_PREFIX=$(CURDIR)/toolchain
	cd toolchain/os-llvm-$(LLVM)/build && ninja
	unset DESTDIR && cd toolchain/os-llvm-$(LLVM)/build && ninja install
clean:
	rm -rf toolchain/os-llvm-$(LLVM) *.tar.gz
.DELETE_ON_ERROR:
-include $(wildcard */*.d) monocypher.d
