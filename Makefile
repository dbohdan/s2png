prefix      = /usr/local

exec_prefix = $(prefix)
bindir      = $(exec_prefix)/bin
datarootdir = $(prefix)/share
datadir     = $(datarootdir)
mandir      = $(datarootdir)/man
man1dir     = $(mandir)/man1

INSTALL         = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA    = $(INSTALL) -m 644

DESTDIR =

CFLAGS  = -g -O2 -Wall
LDFLAGS =
INCS    = `gdlib-config --cflags`
LIBS    = `gdlib-config --libs`

all: test README.md

s2png: s2png.o rc4.o
	$(CC) $(LDFLAGS) $(LIBS) s2png.o rc4.o -o $@

s2png.o: s2png.c
	$(CC) -c $(CFLAGS) $(INCS) $< -o $@

rc4.o: rc4.c
	$(CC) -c $(CFLAGS) $(INCS) $< -o $@

install: installdirs
	$(INSTALL_PROGRAM) s2png $(DESTDIR)$(bindir)/s2png

installdirs:
	mkdir -p $(DESTDIR)$(bindir)

uninstall:
	rm -f $(DESTDIR)$(bindir)/s2png

clean:
	rm -f *.o s2png

# The script uses the output of `s2png -h`, so README.md must have it as
# a prerequisite.
README.md: README.in s2png
	./scripts/gen-readme.sh

test: s2png
	./scripts/test.sh

.PHONY: all install installdirs uninstall clean test
