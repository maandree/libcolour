/* See LICENSE file for copyright and license details. */

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wfloat-equal"
#elif defined(__clang__)
# pragma clang diagnostic ignored "-Wcomma"
# pragma clang diagnostic ignored "-Wfloat-equal"
# pragma clang diagnostic ignored "-Wvla"
# pragma clang diagnostic ignored "-Wtautological-compare"
# pragma clang diagnostic ignored "-Wcovered-switch-default"
# pragma clang diagnostic ignored "-Wfloat-conversion"
# pragma clang diagnostic ignored "-Wconversion"
# pragma clang diagnostic ignored "-Wdouble-promotion"
# pragma clang diagnostic ignored "-Wswitch-enum"
# pragma clang diagnostic ignored "-Wcast-align"
#endif



int
libcolour_convert_en_masse(const libcolour_colour_t *from, const libcolour_colour_t *to,
			   libcolour_convert_en_masse_mode_t mode, size_t n, ...)
{
	libcolour_colour_t tfrom, tto;
	libcolour_convert_en_masse_mode_t alpha_mode = mode & 3;
	int on_cpu = mode & LIBCOLOUR_CONVERT_EN_MASSE_ON_CPU;
	int no_override = mode & LIBCOLOUR_CONVERT_EN_MASSE_NO_OVERRIDE;
	va_list args;
	TYPE *in1, *in2, *in3;
	TYPE *out1, *out2, *out3;
	size_t width;

	if ((unsigned int)mode > 15U) {
		errno = EINVAL;
		return -1;
	}
	if (n < 0) {
		errno = EINVAL;
		return -1;
	}

	va_start(args, n);

	if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_NO_ALPHA) {
		in1 = va_arg(args, TYPE *);
		in2 = in1 + 1;
		in3 = in1 + 2;
		width = 3;
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_FIRST) {
		in1 = va_arg(args, TYPE *);
		in2 = in1 + 2;
		in3 = in1 + 3;
		in1 = in1 + 1;
		width = 4;
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_LAST) {
		in1 = va_arg(args, TYPE *);
		in2 = in1 + 1;
		in3 = in1 + 2;
		width = 4;
	} else {
		in1 = va_arg(args, TYPE *);
		in2 = va_arg(args, TYPE *);
		in3 = va_arg(args, TYPE *);
		width = 1;
	}

	if (!no_override) {
		out1 = in1;
		out2 = in2;
		out3 = in3;
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_NO_ALPHA) {
		out1 = va_arg(args, TYPE *);
		out2 = out1 + 1;
		out3 = out1 + 2;
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_FIRST) {
		out1 = va_arg(args, TYPE *);
		out2 = out1 + 2;
		out3 = out1 + 3;
		out1 = out1 + 1;
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_LAST) {
		out1 = va_arg(args, TYPE *);
		out2 = out1 + 1;
		out3 = out1 + 2;
	} else {
		out1 = va_arg(args, TYPE *);
		out2 = va_arg(args, TYPE *);
		out3 = va_arg(args, TYPE *);
	}

	va_end(args);

	switch (from->model) {
#define X(C, T, N) case C: memcpy(&tfrom, from, sizeof(T));
	LIST_MODELS(X)
#undef X
	default:
		errno = EINVAL;
		return -1;
	}

	switch (to->model) {
#define X(C, T, N) case C: memcpy(&tto, to, sizeof(T));
	LIST_MODELS(X)
#undef X
	default:
		errno = EINVAL;
		return -1;
	}

	while (n--) {
		tfrom.rgb.R = *in1;
		tfrom.rgb.G = *in2;
		tfrom.rgb.B = *in3;
		libcolour_convert(&tfrom, &tto);
		*out1 = tto.rgb.R;
		*out2 = tto.rgb.G;
		*out3 = tto.rgb.B;
		in1 += width;
		in2 += width;
		in3 += width;
		out1 += width;
		out2 += width;
		out3 += width;
	}

	(void) on_cpu;
	return 0;
}
