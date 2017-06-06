.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

SO_VERSION = $(VERSION_MAJOR).$(VERSION_MINOR)

OBJ =\
	float.o\
	double.o\
	long-double.o

HDR =\
	libcolour.h

TEMPLATES =\
	convert-template.c\
	libcolour-template.c


all: libcolour.a libcolour.so.$(SO_VERSION) test


float.o: float.c $(TEMPLATES) $(HDR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ float.c

double.o: double.c $(TEMPLATES) $(HDR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ double.c

long-double.o: long-double.c $(TEMPLATES) $(HDR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ long-double.c


float.pic.o: float.c $(TEMPLATES) $(HDR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -c -o $@ float.c

double.pic.o: double.c $(TEMPLATES) $(HDR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -c -o $@ double.c

long-double.pic.o: long-double.c $(TEMPLATES) $(HDR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -c -o $@ long-double.c


libcolour.a: $(OBJ)
	$(AR) rc $@ $?
	$(AR) -s $@

libcolour.so.$(SO_VERSION): $(OBJ:.o=.pic.o)
	$(CC) -shared -Wl,-soname,libcolour.so.$(VERSION_MAJOR) -o $@ $(LDFLAGS) $^


test.o: test.c libcolour.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ test.c

test: test.o libcolour.a
	$(CC) -o $@ $^ $(LDFLAGS)


install:
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	cp -- libcolour.h "$(DESTDIR)$(PREFIX)/include/libcolour.h"
	chmod -- 644 "$(DESTDIR)$(PREFIX)/include/libcolour.h"
	cp -- libcolour.a "$(DESTDIR)$(PREFIX)/lib/libcolour.a"
	chmod -- 644 "$(DESTDIR)$(PREFIX)/lib/libcolour.a"
	cp -- libcolour.so.$(SO_VERSION) "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(SO_VERSION)"
	chmod -- 755 "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(SO_VERSION)"
	ln -sf -- libcolour.so.$(SO_VERSION) "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(VERSION_MAJOR)"
	ln -sf -- libcolour.so.$(SO_VERSION) "$(DESTDIR)$(PREFIX)/lib/libcolour.so"


uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)/include/libcolour.h"
	-rm -- "$(DESTDIR)$(PREFIX)/lib/libcolour.a"
	-rm -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(SO_VERSION)"
	-rm -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(VERSION_MAJOR)"
	-rm -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so"


check: test
	./test

clean:
	-rm -- *.o *.a *.so *.so.* test

.PHONY: all clean check install uninstall
