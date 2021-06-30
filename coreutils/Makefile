PROGRAMS:=$(patsubst %.c, $(DESTDIR)/bin/%, $(wildcard *.c))

all: $(PROGRAMS)
$(PROGRAMS): $(DESTDIR)/bin/%: %.o
	$(CC) -s -o $@ $^
.DELETE_ON_ERROR:
