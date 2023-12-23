VERSION=0.1.0
CFLAGS?=-g
MAINFLAGS:=-DVERSION='"$(VERSION)"' -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS+=-static
INCLUDE+=-Iinclude
PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin
MANDIR?=$(PREFIX)/share/man
OUTDIR=build
SCDOC=scdoc
.DEFAULT_GOAL=all

OBJECTS=\
	$(OUTDIR)/cical.o \
	$(OUTDIR)/cical_time.o \

$(OUTDIR)/%.o: src/%.c
	@mkdir -p $(OUTDIR)
	$(CC) -std=c11 -pedantic -c -o $@ $(CFLAGS) $(MAINFLAGS) $(INCLUDE) $<

cical: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

cical.1: doc/cical.1.scd
	$(SCDOC) < $< > $@

all: cical cical.1

clean:
	rm -rf $(OUTDIR) cical cical.1

install: all
	mkdir -p $(DESTDIR)/$(BINDIR) $(DESTDIR)/$(MANDIR)/man1
	install -m755 cical $(DESTDIR)/$(BINDIR)/cical
	install -m644 cical.1 $(DESTDIR)/$(MANDIR)/man1/cical.1

uninstall:
	rm -f $(DESTDIR)/$(BINDIR)/cical
	rm -f $(DESTDIR)/$(MANDIR)/man1/cical.1

check: cical cical.1
	@find test -perm -111 -exec '{}' \;

.PHONY: all clean install uninstall check
