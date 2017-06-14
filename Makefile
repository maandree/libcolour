.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

SO_VERSION = $(VERSION_MAJOR).$(VERSION_MINOR)

OBJ =\
	float.o\
	double.o\
	long-double.o

HDR =\
	libcolour.h\
	common.h\
	conversions.h\
	conversion-matrices.h

TEMPLATES =\
	en_masse-template.c\
	libcolour-template.c

MAN7 =\
	libcolour.7\
	LIBCOLOUR_RGB.7\
	LIBCOLOUR_SRGB.7\
	LIBCOLOUR_CIEXYY.7\
	LIBCOLOUR_CIEXYZ.7\
	LIBCOLOUR_CIELAB.7\
	LIBCOLOUR_CIELUV.7\
	LIBCOLOUR_CIELCHUV.7\
	LIBCOLOUR_YIQ.7\
	LIBCOLOUR_YDBDR.7\
	LIBCOLOUR_YUV.7\
	LIBCOLOUR_YPBPR.7\
	LIBCOLOUR_YCGCO.7\
	LIBCOLOUR_CIE1960UCS.7\
	LIBCOLOUR_CIEUVW.7\
	LIBCOLOUR_YES.7

MAN3 =\
	libcolour_convert.3\
	libcolour_srgb_encode.3\
	libcolour_srgb_decode.3\
	libcolour_delta_e.3\
	libcolour_proper.3\
	libcolour_get_rgb_colour_space.3\
	libcolour_marshal.3\
	libcolour_unmarshal.3\
	libcolour_convert_en_masse.3

MAN3_SYMLINKS =\
	libcolour_convert_f.3\
	libcolour_convert_lf.3\
	libcolour_convert_llf.3\
	libcolour_srgb_encode_f.3\
	libcolour_srgb_encode_lf.3\
	libcolour_srgb_encode_llf.3\
	libcolour_srgb_decode_f.3\
	libcolour_srgb_decode_lf.3\
	libcolour_srgb_decode_llf.3\
	libcolour_delta_e_f.3\
	libcolour_delta_e_lf.3\
	libcolour_delta_e_llf.3\
	libcolour_proper_f.3\
	libcolour_proper_lf.3\
	libcolour_proper_llf.3\
	libcolour_get_rgb_colour_space_f.3\
	libcolour_get_rgb_colour_space_lf.3\
	libcolour_get_rgb_colour_space_llf.3\
	libcolour_marshal_f.3\
	libcolour_marshal_lf.3\
	libcolour_marshal_llf.3\
	libcolour_unmarshal_f.3\
	libcolour_unmarshal_lf.3\
	libcolour_unmarshal_llf.3\
	libcolour_convert_en_masse_f.3\
	libcolour_convert_en_masse_lf.3\
	libcolour_convert_en_masse_llf.3


all: libcolour.a libcolour.so.$(SO_VERSION) libcolour.7 test


conversion-matrices.h: matrices.py
	printf '/* This file is generated! */\n' > conversion-matrices.h
	./matrices.py >> conversion-matrices.h


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


libcolour.7: libcolour.7.in
	sed 's:/usr/local:$(PREFIX):g' < libcolour.7.in > libcolour.7


install: libcolour.a libcolour.so.$(SO_VERSION) libcolour.7
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	cp -- libcolour.h "$(DESTDIR)$(PREFIX)/include/libcolour.h"
	chmod -- 644 "$(DESTDIR)$(PREFIX)/include/libcolour.h"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	cp -- libcolour.a "$(DESTDIR)$(PREFIX)/lib/libcolour.a"
	chmod -- 644 "$(DESTDIR)$(PREFIX)/lib/libcolour.a"
	cp -- libcolour.so.$(SO_VERSION) "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(SO_VERSION)"
	chmod -- 755 "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(SO_VERSION)"
	ln -sf -- libcolour.so.$(SO_VERSION) "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(VERSION_MAJOR)"
	ln -sf -- libcolour.so.$(SO_VERSION) "$(DESTDIR)$(PREFIX)/lib/libcolour.so"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -- $(MAN7) "$(DESTDIR)$(MANPREFIX)/man7/"
	cd -- "$(DESTDIR)$(MANPREFIX)/man7" && chmod -- 644 $(MAN7)
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	cp -- $(MAN3) "$(DESTDIR)$(MANPREFIX)/man3/"
	cd -- "$(DESTDIR)$(MANPREFIX)/man3" && chmod -- 644 $(MAN3)
	ln -sf -- libcolour_convert.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_convert_f.3"
	ln -sf -- libcolour_convert.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_convert_lf.3"
	ln -sf -- libcolour_convert.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_convert_llf.3"
	ln -sf -- libcolour_srgb_encode.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_srgb_encode_f.3"
	ln -sf -- libcolour_srgb_encode.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_srgb_encode_lf.3"
	ln -sf -- libcolour_srgb_encode.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_srgb_encode_llf.3"
	ln -sf -- libcolour_srgb_decode.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_srgb_decode_f.3"
	ln -sf -- libcolour_srgb_decode.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_srgb_decode_lf.3"
	ln -sf -- libcolour_srgb_decode.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_srgb_decode_llf.3"
	ln -sf -- libcolour_delta_e.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_delta_e_f.3"
	ln -sf -- libcolour_delta_e.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_delta_e_lf.3"
	ln -sf -- libcolour_delta_e.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_delta_e_llf.3"
	ln -sf -- libcolour_proper.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_proper_f.3"
	ln -sf -- libcolour_proper.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_proper_lf.3"
	ln -sf -- libcolour_proper.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_proper_llf.3"
	ln -sf -- libcolour_get_rgb_colour_space.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_get_rgb_colour_space_f.3"
	ln -sf -- libcolour_get_rgb_colour_space.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_get_rgb_colour_space_lf.3"
	ln -sf -- libcolour_get_rgb_colour_space.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_get_rgb_colour_space_llf.3"
	ln -sf -- libcolour_marshal.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_marshal_f.3"
	ln -sf -- libcolour_marshal.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_marshal_lf.3"
	ln -sf -- libcolour_marshal.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_marshal_llf.3"
	ln -sf -- libcolour_unmarshal.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_unmarshal_f.3"
	ln -sf -- libcolour_unmarshal.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_unmarshal_lf.3"
	ln -sf -- libcolour_unmarshal.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_unmarshal_llf.3"
	ln -sf -- libcolour_convert_en_masse.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_convert_en_masse_f.3"
	ln -sf -- libcolour_convert_en_masse.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_convert_en_masse_lf.3"
	ln -sf -- libcolour_convert_en_masse.3 "$(DESTDIR)$(MANPREFIX)/man3/libcolour_convert_en_masse_llf.3"


uninstall:
	-rm -- "$(DESTDIR)$(PREFIX)/include/libcolour.h"
	-rm -- "$(DESTDIR)$(PREFIX)/lib/libcolour.a"
	-rm -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(SO_VERSION)"
	-rm -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(VERSION_MAJOR)"
	-rm -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so"
	-cd -- "$(DESTDIR)$(MANPREFIX)/man7" && rm -- $(MAN7)
	-cd -- "$(DESTDIR)$(MANPREFIX)/man3" && rm -- $(MAN3) $(MAN3_SYMLINKS)


check: test
	./test

clean:
	-rm -- *.o *.a *.so *.so.* test conversion-matrices.h

.PHONY: all clean check install uninstall
