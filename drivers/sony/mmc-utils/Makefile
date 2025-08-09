CC = gcc
AM_CFLAGS = -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2
CFLAGS = -g -O2
objects = mmc.o mmc_cmds.o

CHECKFLAGS= -Wall -Werror -Wuninitialized -Wundef

DEPFLAGS = -Wp,-MMD,$(@D)/.$(@F).d,-MT,$@

INSTALL = install
prefix ?= /usr/local
bindir = $(prefix)/bin
LIBS=
RESTORE_LIBS=

progs = mmc

# make C=1 to enable sparse
ifdef C
	check = sparse $(CHECKFLAGS)
endif

all: $(progs) manpages

.c.o:
ifdef C
	$(check) $<
endif
	$(CC) $(CHECKFLAGS) $(AM_CFLAGS) $(DEPFLAGS) $(CFLAGS) -c $<

mmc: $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects) $(LDFLAGS) $(LIBS)

manpages:
	cd man && make

install-man:
	cd man && make install

clean:
	rm -f $(progs) $(objects)
	cd man && make clean

install: $(progs) install-man
	$(INSTALL) -m755 -d $(DESTDIR)$(bindir)
	$(INSTALL) $(progs) $(DESTDIR)$(bindir)
