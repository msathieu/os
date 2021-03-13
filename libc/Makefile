CC:=x86_64-os-gcc
AS:=x86_64-os-as
AR:=x86_64-os-ar
CFLAGS:=-ffreestanding -Wall -Wextra -Werror -O2 -MMD -isystem$(DESTDIR)/usr/include

CFILES:=$(wildcard freestanding/*/*.c) $(wildcard */*.c)
SFILES:=$(wildcard os/*.s)
OFILES:=$(patsubst %.c, %.o, $(CFILES)) $(patsubst %.s, %.o, $(SFILES))

all: libc.a crt0.o
libc.a: $(OFILES)
	$(AR) rc libc.a $(OFILES)
install-headers:
	mkdir -p $(DESTDIR)/usr/include/__/freestanding
	cp -r include $(DESTDIR)/usr
	cp freestanding/include/*.h $(DESTDIR)/usr/include/__/freestanding
install: install-headers all
	mkdir -p $(DESTDIR)/lib
	cp libc.a crt0.o $(DESTDIR)/lib
-include $(OFILES:.o=.d)
