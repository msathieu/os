LLVM:=0ece1fdc1d1ba7b5e9504713473f8629e94ff8f9
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
	echo "2f7625d972327882082bef5ada3b080adf06990ebd1a35d68ce78de82e508394b6f49464dc39377c20c86f670719f82ec4715caad6c749dc6593cfadb52bd932  $(LLVM).tar.gz" | shasum -c
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
