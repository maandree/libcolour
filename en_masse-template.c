/* See LICENSE file for copyright and license details. */

#define XPARAMETERS\
	size_t n, size_t width, TYPE *ch1, TYPE *ch2, TYPE *ch3

#define XARGUMENTS\
	(n), (width), (ch1), (ch2), (ch3)

#define PARAMETERS(FROM_SPACE, TO_SPACE)\
	const libcolour_##FROM_SPACE##_t *from, const libcolour_##TO_SPACE##_t *to, XPARAMETERS

#define ARGUMENTS(FROM, TO)\
	(FROM), (TO), XARGUMENTS


#define SIMPLE_LOOP(CH, INSTRUCTIONS)\
	do {\
		TYPE *ch = (CH);\
		size_t i__;\
		for (i__ = 0; i__ < n; i__++) {\
			INSTRUCTIONS;\
			(CH) += width;\
		}\
		(CH) = ch;\
	} while (0)

#define LOOP(INSTRUCTIONS)\
	do {\
		TYPE *c1 = ch1, *c2 = ch2, *c3 = ch3;\
		size_t i__;\
		for (i__ = 0; i__ < n; i__++) {\
			INSTRUCTIONS;\
			ch1 += width;\
			ch2 += width;\
			ch3 += width;\
		}\
		ch1 = c1;\
		ch2 = c2;\
		ch3 = c3;\
	} while (0)

#define CONV_0(MACRO)\
	LOOP(MACRO(*ch1, *ch2, *ch3, *ch1, *ch2, *ch3))

#define CONV_N(MACRO, ...)\
	LOOP(MACRO(*ch1, *ch2, *ch3, *ch1, *ch2, *ch3, __VA_ARGS__))


#define LINEAR_SRGB\
	do {\
		if (from->srgb.with_transfer) {\
			libcolour_srgb_t srgb__ = from->srgb;\
			srgb__.with_transfer = 0;\
			to_srgb(ARGUMENTS(from, &srgb__));\
		}\
	} while (0)



#define X(C, T, N) static void to_##N(const libcolour_colour_t *from, const T *to, XPARAMETERS);
LIST_MODELS(X)
#undef X



static void
conversion_by_matrix(XPARAMETERS, TYPE m11, TYPE m12, TYPE m13, TYPE m21, TYPE m22, TYPE m23, TYPE m31, TYPE m32, TYPE m33)
{
	TYPE c1, c2, c3;
	while (n--) {
		c1 = *ch1;
		c2 = *ch2;
		c3 = *ch3;
		*ch1 = m11 * c1 + m12 * c2 + m13 * c3;
		*ch2 = m21 * c1 + m22 * c2 + m23 * c3;
		*ch3 = m31 * c1 + m32 * c2 + m33 * c3;
		ch1 += width;
		ch2 += width;
		ch3 += width;
	}
}


static void
rgb_encode(XPARAMETERS, const libcolour_rgb_t *restrict space)
{
	TYPE r_sign, g_sign, b_sign;
	switch (space->encoding_type) {
	case LIBCOLOUR_ENCODING_TYPE_LINEAR:
		break;
	case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
	case LIBCOLOUR_ENCODING_TYPE_REGULAR:
		LOOP(do {
				(r_sign = 1, g_sign = 1, b_sign = 1);
				if (*ch1 < 0) (r_sign = -1, *ch1 = -*ch1);
				if (*ch2 < 0) (g_sign = -1, *ch2 = -*ch2);
				if (*ch3 < 0) (b_sign = -1, *ch3 = -*ch3);
				if (space->encoding_type == LIBCOLOUR_ENCODING_TYPE_SIMPLE) {
					*ch1 = xpow(*ch1, 1 / space->GAMMA);
					*ch2 = xpow(*ch2, 1 / space->GAMMA);
					*ch3 = xpow(*ch3, 1 / space->GAMMA);
				} else {
					*ch1 = REGULAR(space, *ch1);
					*ch2 = REGULAR(space, *ch2);
					*ch3 = REGULAR(space, *ch3);
				}
				*ch1 *= r_sign;
				*ch2 *= g_sign;
				*ch3 *= b_sign;
			} while (0));
		break;
	case LIBCOLOUR_ENCODING_TYPE_CUSTOM:
		LOOP((*ch1 = (space->TO_ENCODED_RED)(*ch1),
		      *ch2 = (space->TO_ENCODED_GREEN)(*ch2),
		      *ch3 = (space->TO_ENCODED_BLUE)(*ch3)));
		break;
	default:
		fprintf(stderr, "libcolour: invalid encoding type\n");
		abort();
	}
}

static void
rgb_decode(XPARAMETERS, const libcolour_rgb_t *restrict space)
{
	TYPE r_sign, g_sign, b_sign;
	switch (space->encoding_type) {
	case LIBCOLOUR_ENCODING_TYPE_LINEAR:
		break;
	case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
	case LIBCOLOUR_ENCODING_TYPE_REGULAR:
		LOOP(do {
				(r_sign = 1, g_sign = 1, b_sign = 1);
				if (*ch1 < 0) (r_sign = -1, *ch1 = -*ch1);
				if (*ch2 < 0) (g_sign = -1, *ch2 = -*ch2);
				if (*ch3 < 0) (b_sign = -1, *ch3 = -*ch3);
				if (space->encoding_type == LIBCOLOUR_ENCODING_TYPE_SIMPLE) {
					*ch1 = xpow(*ch1, space->GAMMA);
					*ch2 = xpow(*ch2, space->GAMMA);
					*ch3 = xpow(*ch3, space->GAMMA);
				} else {
					*ch1 = INVREGULAR(space, *ch1);
					*ch2 = INVREGULAR(space, *ch2);
					*ch3 = INVREGULAR(space, *ch3);
				}
				*ch1 *= r_sign;
				*ch2 *= g_sign;
				*ch3 *= b_sign;
			} while (0));
		break;
	case LIBCOLOUR_ENCODING_TYPE_CUSTOM:
		LOOP((*ch1 = (space->TO_DECODED_RED)(*ch1),
		      *ch2 = (space->TO_DECODED_GREEN)(*ch2),
		      *ch3 = (space->TO_DECODED_BLUE)(*ch3)));
		break;
	default:
		fprintf(stderr, "libcolour: invalid encoding type\n");
		abort();
	}
}

static int
rgb_same_transfer(const libcolour_rgb_t *a, const libcolour_rgb_t *b)
{
	if (a->encoding_type != b->encoding_type)
		return 0;
	switch (a->encoding_type) {
	case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
		return a->GAMMA == b->GAMMA;
	case LIBCOLOUR_ENCODING_TYPE_REGULAR:
		return a->GAMMA      == b->GAMMA  &&
		       a->OFFSET     == b->OFFSET &&
		       a->SLOPE      == b->SLOPE  &&
		       a->TRANSITION == b->TRANSITION;
	case LIBCOLOUR_ENCODING_TYPE_CUSTOM:
		return a->TO_ENCODED_RED   == b->TO_ENCODED_RED   &&
		       a->TO_ENCODED_GREEN == b->TO_ENCODED_GREEN &&
		       a->TO_ENCODED_BLUE  == b->TO_ENCODED_BLUE  &&
		       a->TO_DECODED_RED   == b->TO_DECODED_RED   &&
		       a->TO_DECODED_GREEN == b->TO_DECODED_GREEN &&
		       a->TO_DECODED_BLUE  == b->TO_DECODED_BLUE;
	default:
		return 1;
	}
}

static void
to_rgb(PARAMETERS(colour, rgb))
{
	int have_transfer = 0, with_transfer = to->with_transfer;
	switch (from->model) {
	case LIBCOLOUR_RGB:
		if (!memcmp(from->rgb.M, to->M, sizeof(TYPE[3][3]))) {
			have_transfer = from->rgb.with_transfer;
			if (have_transfer && with_transfer && !rgb_same_transfer(&from->rgb, to)) {
				rgb_decode(XARGUMENTS, &from->rgb);
				have_transfer = 0;
			}
			break;
		}
		/* fall through */
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		conversion_by_matrix(XARGUMENTS, CIEXYZ_TO_RGB(to->Minv));
		break;
	}

	if (have_transfer != with_transfer) {
		if (with_transfer)
			rgb_encode(XARGUMENTS, to);
		else
			rgb_decode(XARGUMENTS, &from->rgb);
	}
}


static void
to_srgb(PARAMETERS(colour, srgb))
{
	libcolour_srgb_t tmp;
	switch (from->model) {
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		conversion_by_matrix(XARGUMENTS, CIEXYZ_TO_SRGB);
		break;
	case LIBCOLOUR_SRGB:
	srgb_to_srgb:
		if (from->srgb.with_transfer != to->with_transfer) {
			if (to->with_transfer) {
				LOOP((*ch1 = srgb_encode(*ch1),
				      *ch2 = srgb_encode(*ch2),
				      *ch3 = srgb_encode(*ch3)));
			} else {
				LOOP((*ch1 = srgb_decode(*ch1),
				      *ch2 = srgb_decode(*ch2),
				      *ch3 = srgb_decode(*ch3)));
			}
		}
		return;
	case LIBCOLOUR_YIQ:
		conversion_by_matrix(XARGUMENTS, YIQ_TO_SRGB);
		break;
	case LIBCOLOUR_YDBDR:
		conversion_by_matrix(XARGUMENTS, YDBDR_TO_SRGB);
		break;
	case LIBCOLOUR_YPBPR:
		CONV_0(YPBPR_TO_SRGB);
		break;
	case LIBCOLOUR_YUV:
		conversion_by_matrix(XARGUMENTS, YUV_TO_SRGB);
		break;
	case LIBCOLOUR_YCGCO:
		conversion_by_matrix(XARGUMENTS, YCGCO_TO_SRGB);
		break;
	}
	if (to->with_transfer) {
		tmp = *to;
		tmp.with_transfer = 0;
		from = (const void *)&tmp;
		goto srgb_to_srgb;
	}
}


static void
to_ciexyy(PARAMETERS(colour, ciexyy))
{
	libcolour_srgb_t tmp;
	size_t m, old_n;
beginning:
	m = old_n = n;
	switch (from->model) {
	case LIBCOLOUR_CIEXYY:
		break;
	case LIBCOLOUR_SRGB:
		tmp.model = LIBCOLOUR_SRGB;
		tmp.with_transfer = 0;
		if (from->srgb.with_transfer)
			to_srgb(ARGUMENTS((const libcolour_colour_t *)from, &tmp));
		if (!*ch1 && !*ch2 && !*ch3) {
			for (m = 1; m < n && !ch1[m * width] && !ch2[m * width] && !ch3[m * width]; m++);
			n = m;
			LOOP((*ch1 = D(0.31272660439158),
			      *ch2 = D(0.32902315240275),
			      *ch3 = 0));
			n = old_n;
			break;
		} else {
			for (m = 1; m < n && (ch1[m * width] || ch2[m * width] || ch3[m * width]); m++);
			n = m;
			from = (const void *)&tmp;
		}
		/* fall through */
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_CIEXYY);
		break;
	}
	if (m != old_n) {
		n = old_n - m;
		ch1 += m * width;
		ch2 += m * width;
		ch3 += m * width;
		goto beginning;
	}
}


static void
to_ciexyz(PARAMETERS(colour, ciexyz))
{
	libcolour_colour_t tmp;
	switch (from->model) {
	case LIBCOLOUR_RGB:
		if (from->rgb.with_transfer) {
			tmp.rgb = from->rgb;
			tmp.rgb.with_transfer = 0;
			to_rgb(ARGUMENTS(from, &tmp.rgb));
		}
		conversion_by_matrix(XARGUMENTS, RGB_TO_CIEXYZ(from->rgb.M));
		break;
	default:
		tmp.srgb.model = LIBCOLOUR_SRGB;
		tmp.srgb.with_transfer = 0;
		to_srgb(ARGUMENTS(from, &tmp.srgb));
		from = (const void *)&tmp.srgb;
		/* fall through */
	case LIBCOLOUR_SRGB:
		LINEAR_SRGB;
		conversion_by_matrix(XARGUMENTS, SRGB_TO_CIEXYZ);
		break;
	case LIBCOLOUR_CIEXYY:
		CONV_0(CIEXYY_TO_CIEXYZ);
		break;
	case LIBCOLOUR_CIEXYZ:
		break;
	case LIBCOLOUR_CIELAB:
		CONV_0(CIELAB_TO_CIEXYZ);
		break;
	case LIBCOLOUR_CIELCHUV:
		tmp.cieluv.model = LIBCOLOUR_CIELUV;
		tmp.cieluv.white = from->cielchuv.white;
		to_cieluv(ARGUMENTS(from, &tmp.cieluv));
		from = (const void *)&tmp.cieluv;
		/* fall through */
	case LIBCOLOUR_CIELUV:
		CONV_N(CIELUV_TO_CIEXYZ, from->cieluv.white.X, from->cieluv.white.Y, from->cieluv.white.Z);
		break;
	case LIBCOLOUR_YIQ:
		conversion_by_matrix(XARGUMENTS, YIQ_TO_CIEXYZ);
		break;
	case LIBCOLOUR_YDBDR:
		conversion_by_matrix(XARGUMENTS, YDBDR_TO_CIEXYZ);
		break;
	case LIBCOLOUR_YUV:
		conversion_by_matrix(XARGUMENTS, YUV_TO_CIEXYZ);
		break;
	case LIBCOLOUR_YPBPR:
		conversion_by_matrix(XARGUMENTS, YPBPR_TO_CIEXYZ);
		break;
	case LIBCOLOUR_YCGCO:
		conversion_by_matrix(XARGUMENTS, YCGCO_TO_CIEXYZ);
		break;
	case LIBCOLOUR_CIEUVW:
		tmp.cie1960ucs.model = LIBCOLOUR_CIE1960UCS;
		to_cie1960ucs(ARGUMENTS(from, &tmp.cie1960ucs));
		from = (const void *)&tmp.cie1960ucs;
		/* fall through */
	case LIBCOLOUR_CIE1960UCS:
		CONV_0(CIE1960UCS_TO_CIEXYZ);
		break;
	}
}


static void
to_cielab(PARAMETERS(colour, cielab))
{
	switch (from->model) {
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_CIELAB);
		break;
	case LIBCOLOUR_CIELAB:
		break;
	}
}


static void
to_cieluv(PARAMETERS(colour, cieluv))
{
	libcolour_ciexyz_t tmp;
	libcolour_cielchuv_t tmp2;
	TYPE m;
	switch (from->model) {
	case LIBCOLOUR_CIELCHUV:
		if (to->white.X != from->cielchuv.white.X ||
		    to->white.Y != from->cielchuv.white.Y ||
		    to->white.Z != from->cielchuv.white.Z) {
			tmp.model = LIBCOLOUR_CIEXYZ;
			tmp2.model = LIBCOLOUR_CIELCHUV;
			tmp2.white = to->white;
			tmp2.one_revolution = PI2;
			to_ciexyz(ARGUMENTS(from, &tmp));
			to_cielchuv(ARGUMENTS((const libcolour_colour_t *)&tmp, &tmp2));
		} else if (from->cielchuv.one_revolution != PI2) {
			m = PI2 / from->cielchuv.one_revolution;
			SIMPLE_LOOP(ch3, *ch3 *= m);
		}
		CONV_0(CIELCHUV_TO_CIELUV);
		break;
	case LIBCOLOUR_CIELUV:
		if (to->white.X == from->cieluv.white.X &&
		    to->white.Y == from->cieluv.white.Y &&
		    to->white.Z == from->cieluv.white.Z)
			break;
		/* fall through */
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_N(CIEXYZ_TO_CIELUV, to->white.X, to->white.Y, to->white.Z);
		break;
	}
}


static void
to_cielchuv(PARAMETERS(colour, cielchuv))
{
	libcolour_cieluv_t tmp1, tmp2;
	TYPE m;
	switch (from->model) {
	case LIBCOLOUR_CIELCHUV:
		if (to->white.X == from->cielchuv.white.X &&
		    to->white.Y == from->cielchuv.white.Y &&
		    to->white.Z == from->cielchuv.white.Z) {
			if (to->one_revolution != from->cielchuv.one_revolution) {
				m = to->one_revolution;
				m /= from->cielchuv.one_revolution;
				SIMPLE_LOOP(ch3, *ch3 *= m);
			}
			break;
		}
		/* fall through */
	default:
		tmp1.model = LIBCOLOUR_CIELUV;
		tmp1.white = to->white;
		to_cieluv(ARGUMENTS(from, &tmp1));
		from = (const void *)&tmp1;
		/* fall through */
	case LIBCOLOUR_CIELUV:
		if (to->white.X != from->cieluv.white.X ||
		    to->white.Y != from->cieluv.white.Y ||
		    to->white.Z != from->cieluv.white.Z) {
			tmp2.model = LIBCOLOUR_CIELUV;
			tmp2.white = to->white;
			to_cieluv(ARGUMENTS(from, &tmp2));
			from = (const void *)&tmp2;
		}
		CONV_N(CIELUV_TO_CIELCHUV, to->one_revolution);
		break;
	}
}


static void
to_yiq(PARAMETERS(colour, yiq))
{
	switch (from->model) {
	case LIBCOLOUR_YIQ:
		break;
	case LIBCOLOUR_SRGB:
		LINEAR_SRGB;
		conversion_by_matrix(XARGUMENTS, SRGB_TO_YIQ);
		break;
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		conversion_by_matrix(XARGUMENTS, CIEXYZ_TO_YIQ);
		break;
	}
}


static void
to_ydbdr(PARAMETERS(colour, ydbdr))
{
	switch (from->model) {
	case LIBCOLOUR_YDBDR:
		break;
	case LIBCOLOUR_SRGB:
		LINEAR_SRGB;
		conversion_by_matrix(XARGUMENTS, SRGB_TO_YDBDR);
		break;
	case LIBCOLOUR_YUV:
		CONV_0(YUV_TO_YDBDR);
		break;
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		conversion_by_matrix(XARGUMENTS, CIEXYZ_TO_YDBDR);
		break;
	}
}


static void
to_yuv(PARAMETERS(colour, yuv))
{
	switch (from->model) {
	case LIBCOLOUR_YUV:
		break;
	case LIBCOLOUR_YDBDR:
		CONV_0(YDBDR_TO_YUV);
		break;
	case LIBCOLOUR_SRGB:
		LINEAR_SRGB;
		conversion_by_matrix(XARGUMENTS, SRGB_TO_YUV);
		break;
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		conversion_by_matrix(XARGUMENTS, CIEXYZ_TO_YUV);
		break;
	}
}


static void
to_ypbpr(PARAMETERS(colour, ypbpr))
{
	switch (from->model) {
	case LIBCOLOUR_YPBPR:
		break;
	case LIBCOLOUR_SRGB:
		LINEAR_SRGB;
		CONV_0(SRGB_TO_YPBPR);
		break;
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		conversion_by_matrix(XARGUMENTS, CIEXYZ_TO_YPBPR);
		break;
	}
}


static void
to_ycgco(PARAMETERS(colour, ycgco))
{
	switch (from->model) {
	case LIBCOLOUR_YCGCO:
		break;
	case LIBCOLOUR_SRGB:
		LINEAR_SRGB;
		conversion_by_matrix(XARGUMENTS, SRGB_TO_YCGCO);
		break;
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		conversion_by_matrix(XARGUMENTS, CIEXYZ_TO_YCGCO);
		break;
	}
}


static void
to_cie1960ucs(PARAMETERS(colour, cie1960ucs))
{
	switch (from->model) {
	case LIBCOLOUR_CIE1960UCS:
		break;
	case LIBCOLOUR_CIEUVW:
		CONV_N(CIEUVW_TO_CIE1960UCS, from->cieuvw.u0, from->cieuvw.v0);
		break;
	default:
		to_ciexyz(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_CIE1960UCS);
		break;
	}
}


static void
to_cieuvw(PARAMETERS(colour, cieuvw))
{
	switch (from->model) {
	case LIBCOLOUR_CIEUVW:
		CONV_N(CIEUVW_TO_CIEUVW, from->cieuvw.u0, from->cieuvw.v0, to->u0, to->v0);
		break;
	default:
		to_cie1960ucs(ARGUMENTS(from, NULL));
		/* fall through */
	case LIBCOLOUR_CIE1960UCS:
		CONV_N(CIE1960UCS_TO_CIEUVW, to->u0, to->v0);
		break;
	}
}


int
libcolour_convert_en_masse(const libcolour_colour_t *from, const libcolour_colour_t *to,
			   libcolour_convert_en_masse_mode_t mode, size_t n, ...)
{
	libcolour_colour_t tfrom, tto;
	libcolour_convert_en_masse_mode_t alpha_mode = mode & 3;
	int on_cpu = mode & LIBCOLOUR_CONVERT_EN_MASSE_ON_CPU;
	int no_override = mode & LIBCOLOUR_CONVERT_EN_MASSE_NO_OVERRIDE;
	va_list args;
	TYPE *in1, *in2, *in3, *in_alpha = NULL;
	TYPE *ch1, *ch2, *ch3, *ch_alpha = NULL;
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
		in_alpha = va_arg(args, TYPE *);
		in1 = in_alpha + 1;
		in2 = in_alpha + 2;
		in3 = in_alpha + 3;
		width = 4;
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_LAST) {
		in1 = va_arg(args, TYPE *);
		in2 = in1 + 1;
		in3 = in1 + 2;
		in_alpha = in1 + 3;
		width = 4;
	} else {
		in1 = va_arg(args, TYPE *);
		in2 = va_arg(args, TYPE *);
		in3 = va_arg(args, TYPE *);
		width = 1;
	}

	if (!no_override) {
		ch1 = in1;
		ch2 = in2;
		ch3 = in3;
		ch_alpha = in_alpha;
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_NO_ALPHA) {
		ch1 = va_arg(args, TYPE *);
		ch2 = ch1 + 1;
		ch3 = ch1 + 2;
		memcpy(ch1, in1, n * 3 * sizeof(TYPE));
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_FIRST) {
		ch_alpha = va_arg(args, TYPE *);
		ch1 = ch_alpha + 1;
		ch2 = ch_alpha + 2;
		ch3 = ch_alpha + 3;
		memcpy(ch_alpha, in_alpha, n * 4 * sizeof(TYPE));
	} else if (alpha_mode == LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_LAST) {
		ch1 = va_arg(args, TYPE *);
		ch2 = ch1 + 1;
		ch3 = ch1 + 2;
		ch_alpha = ch1 + 3;
		memcpy(ch1, in1, n * 4 * sizeof(TYPE));
	} else {
		ch1 = va_arg(args, TYPE *);
		ch2 = va_arg(args, TYPE *);
		ch3 = va_arg(args, TYPE *);
		memcpy(ch1, in1, n * sizeof(TYPE));
		memcpy(ch2, in2, n * sizeof(TYPE));
		memcpy(ch3, in3, n * sizeof(TYPE));
	}

	va_end(args);

	switch (from->model) {
#define X(C, T, N) case C: memcpy(&tfrom, from, sizeof(T)); break;
	LIST_MODELS(X)
#undef X
	default:
		errno = EINVAL;
		return -1;
	}

	switch (to->model) {
#define X(C, T, N)\
		case C:\
			memcpy(&tto, to, sizeof(T));\
			to_##N(ARGUMENTS(&tfrom, &tto.N));\
			break;
		LIST_MODELS(X)
#undef X
	default:
		errno = EINVAL;
		return -1;
	}

	(void) on_cpu;
	return 0;
}
