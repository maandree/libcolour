.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows
include mk/$(OS).mk


LIB_MAJOR = 2
LIB_MINOR = 3
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)



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


all: libcolour.a libcolour.$(LIBEXT) test
$(OBJ): $(HDR) $(TEMPLATES)
$(LOBJ): $(HDR) $(TEMPLATES)
test.o: test.c libcolour.h

conversion-matrices.h: matrices.py
	printf '/* This file is generated! */\n' > conversion-matrices.h
	./matrices.py >> conversion-matrices.h

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

test: test.o libcolour.a
	$(CC) -o $@ test.o libcolour.a $(LDFLAGS)

libcolour.$(lIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) -o $@ $(LOBJ) $(LDFLAGS)

libcolour.a: $(OBJ)
	-rm -f -- $@
	$(AR) rc $@ $(OBJ)
	$(AR) -s $@

check: test
	./test

install: libcolour.a libcolour.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -- libcolour.h "$(DESTDIR)$(PREFIX)/include/libcolour.h"
	cp -- libcolour.a "$(DESTDIR)$(PREFIX)/lib/libcolour.a"
	cp -- libcolour.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libcolour.$(LIBMINOREXT)"
	$(FIX_INSTALL_NAME) -- "$(DESTDIR)$(PREFIX)/lib/libcolour.$(LIBMINOREXT)"
	ln -sf -- "libcolour.$(LIBMINOREXT)" "$(DESTDIR)$(PREFIX)/lib/libcolour.$(LIBMAJOREXT)"
	ln -sf -- "libcolour.$(LIBMAJOREXT)" "$(DESTDIR)$(PREFIX)/lib/libcolour.$(LIBEXT)"
	cp -- $(MAN3) "$(DESTDIR)$(MANPREFIX)/man3/"
	cp -- $(MAN7) "$(DESTDIR)$(MANPREFIX)/man7/"
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
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libcolour.h"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcolour.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(LIBEXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libcolour.so.$(LIBMINOREXT)"
	-cd -- "$(DESTDIR)$(MANPREFIX)/man7" && rm -f -- $(MAN7)
	-cd -- "$(DESTDIR)$(MANPREFIX)/man3" && rm -f -- $(MAN3) $(MAN3_SYMLINKS)

clean:
	-rm -f -- *.o *.lo *.a *.so *.so.* test conversion-matrices.h
	-rm -rf -- __pycache__/

.SUFFIXES:
.SUFFIXES: .lo .o .c

.PHONY: all check install uninstall clean
