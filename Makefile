VERSION=0.1.0
CFLAGS?=-g
MAINFLAGS:=-DVERSION='"$(VERSION)"' -std=c11 -pedantic -Wall -Wextra -Werror -Wno-unused-parameter
INCLUDE+=-Iinclude
PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin
MANDIR?=$(PREFIX)/share/man
SCDOC=scdoc
.DEFAULT_GOAL=all

#
# Project files
#
SRCDIR=src
SRCS=\
	cical.c \
	cical_time.c \
	cical_list.c \
	cical_reader.c \
	cical_print_json.c \
	cical_print_markdown.c
OBJS=$(SRCS:.c=.o)
EXE=cical

#
# Debug build settings
#
DBGDIR=debug
DBGEXE=$(DBGDIR)/$(EXE)
DBGOBJS=$(addprefix $(DBGDIR)/, $(OBJS))
DBGCFLAGS=-g $(CFLAGS)

#
# Release build settings
#
RELDIR=build
RELEXE=$(RELDIR)/$(EXE)
RELOBJS=$(addprefix $(RELDIR)/, $(OBJS))
RELCFLAGS=-DNDEBUG $(CFLAGS)

#
# Default build
#
all: release

#
# Debug rules
#
debug: $(DBGEXE) cical.1

$(DBGEXE): $(DBGOBJS)
	$(CC) $(DBGCFLAGS) $(MAINFLAGS) $(LDFLAGS) -o $@ $^

$(DBGDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(DBGDIR)
	$(CC) -c $(DBGCFLAGS) $(MAINFLAGS) $(INCLUDE) -o $@ $<

#
# Release rules
#
release: $(RELEXE) cical.1

$(RELEXE): $(RELOBJS)
	$(CC) $(RELCFLAGS) $(MAINFLAGS) $(LDFLAGS) -o $@ $^

$(RELDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(RELDIR)
	$(CC) -c $(RELCFLAGS) $(MAINFLAGS) $(INCLUDE) -o $@ $<

#
# Other rules
#
cical.1: doc/cical.1.scd
	$(SCDOC) < $< > $@

clean:
	rm -rf $(RELDIR) $(DBGDIR)

remake: clean all

install: all
	mkdir -p $(BINDIR) $(MANDIR)/man1
	install -m755 $(RELDIR)/cical $(BINDIR)/cical
	install -m644 cical.1 $(MANDIR)/man1/cical.1

uninstall:
	rm -f $(BINDIR)/cical
	rm -f $(MANDIR)/man1/cical.1

tags: all
	ctags -R

tests: debug
	test/test.sh

memcheck: debug
	TEST_PREFIX="valgrind -s --leak-check=full --error-exitcode=1 --track-origins=yes" test/test.sh

.PHONY: all clean remake install uninstall test debug release
