PROGRAMS:=$(patsubst %.c, $(DESTDIR)/bin/%, $(wildcard *.c))

$(PROGRAMS): $(DESTDIR)/bin/%: %.o
	$(CC) -s -o $@ $<
