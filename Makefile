VERSION=0.1.0
CFLAGS?=-g
MAINFLAGS:=-DVERSION='"$(VERSION)"' -Wall -Wextra -Werror -Wno-unused-parameter
AUXFLAGS?=-DNDEBUG
#LDFLAGS+=-static
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
	$(OUTDIR)/cical_list.o \
	$(OUTDIR)/cical_reader.o \
	$(OUTDIR)/cical_print_json.o

$(OUTDIR)/%.o: src/%.c
	@mkdir -p $(OUTDIR)
	$(CC) -std=c11 -pedantic -c -o $@ $(CFLAGS) $(MAINFLAGS) $(AUXFLAGS) $(INCLUDE) $<

cical: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

cical.1: doc/cical.1.scd
	$(SCDOC) < $< > $@

all: cical cical.1

clean:
	rm -rf $(OUTDIR) cical cical.1

install: all
	mkdir -p $(BINDIR) $(MANDIR)/man1
	install -m755 cical $(BINDIR)/cical
	install -m644 cical.1 $(MANDIR)/man1/cical.1

uninstall:
	rm -f $(BINDIR)/cical
	rm -f $(MANDIR)/man1/cical.1

check: cical cical.1
	@find test -perm -111 -exec '{}' \;

tests: all test/test.sh
	test/test.sh

memcheck: all test/test.sh
	TEST_PREFIX="valgrind -s --leak-check=full --error-exitcode=1 --track-origins=yes" test/test.sh

.PHONY: all clean install uninstall check test
