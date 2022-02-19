PREFIX    = /usr
MANPREFIX = $(PREFIX)/share/man

CC = cc

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE
CFLAGS   = -std=c11 -Wall -pedantic -O2
LDFLAGS  = -lm -s
