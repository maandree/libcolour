.POSIX:

include config.mk

OBJ =\
	float.o\
	double.o\
	long-double.o

HDR =\
	libcolour.h

TEMPLATES =\
	convert-template.c\
	libcolour-template.c


all: libcolour.a libcolour.so.$(VERSION_MAJOR).$(VERSION_MINOR) test


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

libcolour.so.$(VERSION_MAJOR).$(VERSION_MINOR): $(OBJ:.o=.pic.o)
	$(CC) -shared -Wl,-soname,libcolour.so.$(VERSION_MAJOR) -o $@ $(LDFLAGS) $^


test.o: test.c libcolour.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ test.c

test: test.o libcolour.a
	$(CC) -o $@ $^ $(LDFLAGS)


check: test
	./test

clean:
	-rm -- *.o *.a *.so *.so.* test

.PHONY: all clean check
