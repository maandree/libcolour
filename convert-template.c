/* See LICENSE file for copyright and license details. */

static void to_rgb(const libcolour_colour_t *restrict from, libcolour_rgb_t *restrict to);
static void to_srgb(const libcolour_colour_t *restrict from, libcolour_srgb_t *restrict to);
static void to_ciexyy(const libcolour_colour_t *restrict from, libcolour_ciexyy_t *restrict to);
static void to_ciexyz(const libcolour_colour_t *restrict from, libcolour_ciexyz_t *restrict to);
static void to_cielab(const libcolour_colour_t *restrict from, libcolour_cielab_t *restrict to);
static void to_cieluv(const libcolour_colour_t *restrict from, libcolour_cieluv_t *restrict to);
static void to_cielchuv(const libcolour_colour_t *restrict from, libcolour_cielchuv_t *restrict to);
static void to_yiq(const libcolour_colour_t *restrict from, libcolour_yiq_t *restrict to);
static void to_ydbdr(const libcolour_colour_t *restrict from, libcolour_ydbdr_t *restrict to);
static void to_yuv(const libcolour_colour_t *restrict from, libcolour_yuv_t *restrict to);
static void to_ypbpr(const libcolour_colour_t *restrict from, libcolour_ypbpr_t *restrict to);
static void to_ycgco(const libcolour_colour_t *restrict from, libcolour_ycgco_t *restrict to);
static void to_cie1960ucs(const libcolour_colour_t *restrict from, libcolour_cie1960ucs_t *restrict to);
static void to_cieuvw(const libcolour_colour_t *restrict from, libcolour_cieuvw_t *restrict to);


#define CONV_0(MACRO, FROM, TO)\
	MACRO(((const libcolour_rgb_t *)(FROM))->R,\
	      ((const libcolour_rgb_t *)(FROM))->G,\
	      ((const libcolour_rgb_t *)(FROM))->B,\
	      ((libcolour_rgb_t *)(TO))->R,\
	      ((libcolour_rgb_t *)(TO))->G,\
	      ((libcolour_rgb_t *)(TO))->B)

#define CONV_N(MACRO, FROM, TO, ...)\
	MACRO(((const libcolour_rgb_t *)(FROM))->R,\
	      ((const libcolour_rgb_t *)(FROM))->G,\
	      ((const libcolour_rgb_t *)(FROM))->B,\
	      ((libcolour_rgb_t *)(TO))->R,\
	      ((libcolour_rgb_t *)(TO))->G,\
	      ((libcolour_rgb_t *)(TO))->B,\
	      __VA_ARGS__)


static void
rgb_encode(libcolour_rgb_t *restrict colour, const libcolour_rgb_t *restrict space)
{
	TYPE r_sign = 1, g_sign = 1, b_sign = 1;
	switch (space->encoding_type) {
	case LIBCOLOUR_ENCODING_TYPE_LINEAR:
		break;
	case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
	case LIBCOLOUR_ENCODING_TYPE_REGULAR:
		if (colour->R < 0)  r_sign = -1, colour->R = -colour->R;
		if (colour->G < 0)  g_sign = -1, colour->G = -colour->G;
		if (colour->B < 0)  b_sign = -1, colour->B = -colour->B;
		if (space->encoding_type == LIBCOLOUR_ENCODING_TYPE_SIMPLE) {
			colour->R = xpow(colour->R, 1 / space->GAMMA);
			colour->G = xpow(colour->G, 1 / space->GAMMA);
			colour->B = xpow(colour->B, 1 / space->GAMMA);
		} else {
			colour->R = REGULAR(space, colour->R);
			colour->G = REGULAR(space, colour->G);
			colour->B = REGULAR(space, colour->B);
		}
		colour->R *= r_sign;
		colour->G *= g_sign;
		colour->B *= b_sign;
		break;
	case LIBCOLOUR_ENCODING_TYPE_CUSTOM:
		colour->R = (space->TO_ENCODED_RED)(colour->R);
		colour->G = (space->TO_ENCODED_GREEN)(colour->G);
		colour->B = (space->TO_ENCODED_BLUE)(colour->B);
		break;
	default:
		fprintf(stderr, "libcolour: invalid encoding type\n");
		abort();
	}
}

static void
rgb_decode(libcolour_rgb_t *restrict colour, const libcolour_rgb_t *restrict space)
{
	TYPE r_sign = 1, g_sign = 1, b_sign = 1;
	switch (space->encoding_type) {
	case LIBCOLOUR_ENCODING_TYPE_LINEAR:
		break;
	case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
	case LIBCOLOUR_ENCODING_TYPE_REGULAR:
		if (colour->R < 0)  r_sign = -1, colour->R = -colour->R;
		if (colour->G < 0)  g_sign = -1, colour->G = -colour->G;
		if (colour->B < 0)  b_sign = -1, colour->B = -colour->B;
		if (space->encoding_type == LIBCOLOUR_ENCODING_TYPE_SIMPLE) {
			colour->R = xpow(colour->R, space->GAMMA);
			colour->G = xpow(colour->G, space->GAMMA);
			colour->B = xpow(colour->B, space->GAMMA);
		} else {
			colour->R = INVREGULAR(space, colour->R);
			colour->G = INVREGULAR(space, colour->G);
			colour->B = INVREGULAR(space, colour->B);
		}
		colour->R *= r_sign;
		colour->G *= g_sign;
		colour->B *= b_sign;
		break;
	case LIBCOLOUR_ENCODING_TYPE_CUSTOM:
		colour->R = (space->TO_DECODED_RED)(colour->R);
		colour->G = (space->TO_DECODED_GREEN)(colour->G);
		colour->B = (space->TO_DECODED_BLUE)(colour->B);
		break;
	default:
		fprintf(stderr, "libcolour: invalid encoding type\n");
		abort();
	}
}

static int
rgb_same_transfer(const libcolour_rgb_t *restrict a, const libcolour_rgb_t *restrict b)
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
to_rgb(const libcolour_colour_t *restrict from, libcolour_rgb_t *restrict to)
{
	int have_transfer = 0, with_transfer = to->with_transfer;
	libcolour_ciexyz_t tmp;
	switch (from->model) {
	case LIBCOLOUR_RGB:
		if (!memcmp(from->rgb.M, to->M, sizeof(TYPE[3][3]))) {
			have_transfer = from->rgb.with_transfer;
			to->R = from->rgb.R;
			to->G = from->rgb.G;
			to->B = from->rgb.B;
			if (have_transfer && with_transfer && !rgb_same_transfer(&from->rgb, to))
				rgb_decode(to, &from->rgb), have_transfer = 0;
			break;
		}
		/* fall through */
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp);
		from = (const void *)&tmp;
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_N(CIEXYZ_TO_RGB, from, to, to->Minv);
		break;
	}

	if (have_transfer != with_transfer) {
		if (with_transfer)
			rgb_encode(to, to);
		else
			rgb_decode(to, &from->rgb);
	}
}


static void
to_srgb(const libcolour_colour_t *restrict from, libcolour_srgb_t *restrict to)
{
	libcolour_srgb_t tmp_srgb;
	libcolour_ciexyz_t tmp_ciexyz;
	switch (from->model) {
	default:
		tmp_ciexyz.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp_ciexyz);
		from = (const void *)&tmp_ciexyz;
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_SRGB, from, to);
		break;
	case LIBCOLOUR_SRGB:
	srgb_to_srgb:
		if (from->srgb.with_transfer == to->with_transfer) {
			*to = from->srgb;
		} else if (to->with_transfer) {
			to->R = srgb_encode(from->srgb.R);
			to->G = srgb_encode(from->srgb.G);
			to->B = srgb_encode(from->srgb.B);
		} else {
			to->R = srgb_decode(from->srgb.R);
			to->G = srgb_decode(from->srgb.G);
			to->B = srgb_decode(from->srgb.B);
		}
		return;
	case LIBCOLOUR_YIQ:
		CONV_0(YIQ_TO_SRGB, from, to);
		break;
	case LIBCOLOUR_YDBDR:
		CONV_0(YDBDR_TO_SRGB, from, to);
		break;
	case LIBCOLOUR_YPBPR:
		CONV_0(YPBPR_TO_SRGB, from, to);
		break;
	case LIBCOLOUR_YUV:
		CONV_0(YUV_TO_SRGB, from, to);
		break;
	case LIBCOLOUR_YCGCO:
		CONV_0(YCGCO_TO_SRGB, from, to);
		break;
	}
	if (to->with_transfer) {
		tmp_srgb = *to;
		tmp_srgb.with_transfer = 0;
		from = (const void *)&tmp_srgb;
		goto srgb_to_srgb;
	}
}


static void
to_ciexyy(const libcolour_colour_t *restrict from, libcolour_ciexyy_t *restrict to)
{
	libcolour_srgb_t tmp1;
	libcolour_ciexyz_t tmp2;
	switch (from->model) {
	case LIBCOLOUR_CIEXYY:
		*to = from->ciexyy;
		break;
	case LIBCOLOUR_SRGB:
		tmp1.model = LIBCOLOUR_SRGB;
		if (from->srgb.with_transfer) {
			tmp1.with_transfer = 0;
			to_srgb((const libcolour_colour_t *)from, &tmp1);
		} else {
			tmp1 = from->srgb;
		}
		if (tmp1.R == 0 && tmp1.G == 0 && tmp1.B == 0) {
			to->x = D(0.31272660439158);
			to->y = D(0.32902315240275);
			to->Y = 0;
			break;
		} else {
			from = (const void *)&tmp1;
		}
		/* fall through */
	default:
		tmp2.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp2);
		from = (const void *)&tmp2;
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_CIEXYY, from, to);
		break;
	}
}


static void
to_ciexyz(const libcolour_colour_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	libcolour_colour_t tmp;
	libcolour_srgb_t tmp_srgb;
	TYPE R, G, B;
	switch (from->model) {
	case LIBCOLOUR_RGB:
		if (from->rgb.with_transfer) {
			tmp.rgb = from->rgb;
			tmp.rgb.with_transfer = 0;
			to_rgb(from, &tmp.rgb);
			R = tmp.rgb.R, G = tmp.rgb.G, B = tmp.rgb.B;
		} else {
			R = from->rgb.R, G = from->rgb.G, B = from->rgb.B;
		}
		RGB_TO_CIEXYZ(R, G, B, to->X, to->Y, to->Z, from->rgb.M);
		break;
	default:
		tmp.srgb.model = LIBCOLOUR_SRGB;
		tmp.srgb.with_transfer = 0;
		to_srgb(from, &tmp.srgb);
		from = (const void *)&tmp.srgb;
		/* fall through */
	case LIBCOLOUR_SRGB:
		if (from->srgb.with_transfer) {
			tmp_srgb.model = LIBCOLOUR_SRGB;
			tmp_srgb.with_transfer = 0;
			to_srgb(from, &tmp_srgb);
			R = tmp_srgb.R, G = tmp_srgb.G, B = tmp_srgb.B;
		} else {
			R = from->srgb.R, G = from->srgb.G, B = from->srgb.B;
		}
		SRGB_TO_CIEXYZ(R, G, B, to->X, to->Y, to->Z);
		break;
	case LIBCOLOUR_CIEXYY:
		CONV_0(CIEXYY_TO_CIEXYZ, from, to);
		break;
	case LIBCOLOUR_CIEXYZ:
		*to = from->ciexyz;
		break;
	case LIBCOLOUR_CIELAB:
		CONV_0(CIELAB_TO_CIEXYZ, from, to);
		break;
	case LIBCOLOUR_CIELCHUV:
		tmp.cieluv.model = LIBCOLOUR_CIELUV;
		tmp.cieluv.white = from->cielchuv.white;
		to_cieluv(from, &tmp.cieluv);
		from = (const void *)&tmp.cieluv;
		/* fall through */
	case LIBCOLOUR_CIELUV:
		CONV_N(CIELUV_TO_CIEXYZ, from, to, from->cieluv.white.X, from->cieluv.white.Y, from->cieluv.white.Z);
		break;
	case LIBCOLOUR_YIQ:
		CONV_0(YIQ_TO_CIEXYZ, from, to);
		break;
	case LIBCOLOUR_YDBDR:
		CONV_0(YDBDR_TO_CIEXYZ, from, to);
		break;
	case LIBCOLOUR_YUV:
		CONV_0(YUV_TO_CIEXYZ, from, to);
		break;
	case LIBCOLOUR_YPBPR:
		CONV_0(YPBPR_TO_CIEXYZ, from, to);
		break;
	case LIBCOLOUR_YCGCO:
		CONV_0(YCGCO_TO_CIEXYZ, from, to);
		break;
	case LIBCOLOUR_CIEUVW:
		tmp.cie1960ucs.model = LIBCOLOUR_CIE1960UCS;
		to_cie1960ucs(from, &tmp.cie1960ucs);
		from = (const void *)&tmp.cie1960ucs;
		/* fall through */
	case LIBCOLOUR_CIE1960UCS:
		CONV_0(CIE1960UCS_TO_CIEXYZ, from, to);
		break;
	}
}


static void
to_cielab(const libcolour_colour_t *restrict from, libcolour_cielab_t *restrict to)
{
	libcolour_ciexyz_t tmp;
	switch (from->model) {
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp);
		from = (const void *)&tmp;
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_CIELAB, from, to);
		break;
	case LIBCOLOUR_CIELAB:
		*to = from->cielab;
		break;
	}
}


static void
to_cieluv(const libcolour_colour_t *restrict from, libcolour_cieluv_t *restrict to)
{
	libcolour_ciexyz_t tmp;
	libcolour_cielchuv_t tmp2;
	TYPE L, C, h;
	switch (from->model) {
	case LIBCOLOUR_CIELCHUV:
		if (to->white.X != from->cielchuv.white.X ||
		    to->white.Y != from->cielchuv.white.Y ||
		    to->white.Z != from->cielchuv.white.Z) {
			tmp.model = LIBCOLOUR_CIEXYZ;
			tmp2.model = LIBCOLOUR_CIELCHUV;
			tmp2.white = to->white;
			tmp2.one_revolution = PI2;
			to_ciexyz(from, &tmp);
			to_cielchuv((const libcolour_colour_t *)&tmp, &tmp2);
			L = tmp2.L, C = tmp2.C, h = tmp2.h;
		} else {
			L = from->cielchuv.L;
			C = from->cielchuv.C;
			h = from->cielchuv.h * PI2 / from->cielchuv.one_revolution;
		}
		CIELCHUV_TO_CIELUV(L, C, h, to->L, to->u, to->v);
		break;
	case LIBCOLOUR_CIELUV:
		if (to->white.X == from->cieluv.white.X &&
		    to->white.Y == from->cieluv.white.Y &&
		    to->white.Z == from->cieluv.white.Z) {
			*to = from->cieluv;
			break;
		}
		/* fall through */
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp);
		from = (const void *)&tmp;
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_N(CIEXYZ_TO_CIELUV, from, to, to->white.X, to->white.Y, to->white.Z);
		break;
	}
}


static void
to_cielchuv(const libcolour_colour_t *restrict from, libcolour_cielchuv_t *restrict to)
{
	libcolour_cieluv_t tmp1, tmp2;
	TYPE one_revolution;
	switch (from->model) {
	case LIBCOLOUR_CIELCHUV:
		if (to->white.X == from->cielchuv.white.X &&
		    to->white.Y == from->cielchuv.white.Y &&
		    to->white.Z == from->cielchuv.white.Z) {
			if (to->one_revolution == from->cielchuv.one_revolution) {
				*to = from->cielchuv;
			} else {
				one_revolution = to->one_revolution;
				*to = from->cielchuv;
				to->one_revolution = one_revolution;
				to->h = to->h / from->cielchuv.one_revolution * one_revolution;
			}
			break;
		}
		/* fall through */
	default:
		tmp1.model = LIBCOLOUR_CIELUV;
		tmp1.white = to->white;
		to_cieluv(from, &tmp1);
		from = (const void *)&tmp1;
		/* fall through */
	case LIBCOLOUR_CIELUV:
		if (to->white.X != from->cieluv.white.X ||
		    to->white.Y != from->cieluv.white.Y ||
		    to->white.Z != from->cieluv.white.Z) {
			tmp2.model = LIBCOLOUR_CIELUV;
			tmp2.white = to->white;
			to_cieluv(from, &tmp2);
			from = (const void *)&tmp2;
		}
		CONV_N(CIELUV_TO_CIELCHUV, from, to, to->one_revolution);
		break;
	}
}


static void
to_yiq(const libcolour_colour_t *restrict from, libcolour_yiq_t *restrict to)
{
	libcolour_colour_t tmp = *from;
	switch (from->model) {
	case LIBCOLOUR_YIQ:
		*to = from->yiq;
		break;
	case LIBCOLOUR_SRGB:
		if (tmp.srgb.with_transfer) {
			tmp.srgb.with_transfer = 0;
			to_srgb(from, &tmp.srgb);
		}
		CONV_0(SRGB_TO_YIQ, &tmp, to);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_YIQ, &tmp, to);
		break;
	}
}


static void
to_ydbdr(const libcolour_colour_t *restrict from, libcolour_ydbdr_t *restrict to)
{
	libcolour_colour_t tmp = *from;
	switch (from->model) {
	case LIBCOLOUR_YDBDR:
		*to = from->ydbdr;
		break;
	case LIBCOLOUR_SRGB:
		if (tmp.srgb.with_transfer) {
			tmp.srgb.with_transfer = 0;
			to_srgb(from, &tmp.srgb);
		}
		CONV_0(SRGB_TO_YDBDR, &tmp, to);
		break;
	case LIBCOLOUR_YUV:
		CONV_0(YUV_TO_YDBDR, &tmp, to);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_YDBDR, &tmp, to);
		break;
	}
}


static void
to_yuv(const libcolour_colour_t *restrict from, libcolour_yuv_t *restrict to)
{
	libcolour_colour_t tmp = *from;
	switch (from->model) {
	case LIBCOLOUR_YUV:
		*to = from->yuv;
		break;
	case LIBCOLOUR_YDBDR:
		CONV_0(YDBDR_TO_YUV, from, to);
		break;
	case LIBCOLOUR_SRGB:
		if (tmp.srgb.with_transfer) {
			tmp.srgb.with_transfer = 0;
			to_srgb(from, &tmp.srgb);
		}
		CONV_0(SRGB_TO_YUV, &tmp, to);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_YUV, &tmp, to);
		break;
	}
}


static void
to_ypbpr(const libcolour_colour_t *restrict from, libcolour_ypbpr_t *restrict to)
{
	libcolour_colour_t tmp = *from;
	switch (from->model) {
	case LIBCOLOUR_YPBPR:
		*to = from->ypbpr;
		break;
	case LIBCOLOUR_SRGB:
		if (tmp.srgb.with_transfer) {
			tmp.srgb.with_transfer = 0;
			to_srgb(from, &tmp.srgb);
		}
		CONV_0(SRGB_TO_YPBPR, &tmp, to);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_YPBPR, &tmp, to);
		break;
	}
}


static void
to_ycgco(const libcolour_colour_t *restrict from, libcolour_ycgco_t *restrict to)
{
	libcolour_colour_t tmp = *from;
	switch (from->model) {
	case LIBCOLOUR_YCGCO:
		*to = from->ycgco;
		break;
	case LIBCOLOUR_SRGB:
		if (tmp.srgb.with_transfer) {
			tmp.srgb.with_transfer = 0;
			to_srgb(from, &tmp.srgb);
		}
		CONV_0(SRGB_TO_YCGCO, &tmp, to);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_YCGCO, &tmp, to);
		break;
	}
}


static void
to_cie1960ucs(const libcolour_colour_t *restrict from, libcolour_cie1960ucs_t *restrict to)
{
	libcolour_colour_t tmp = *from;
	switch (from->model) {
	case LIBCOLOUR_CIE1960UCS:
		*to = from->cie1960ucs;
		return;
	case LIBCOLOUR_CIEUVW:
		CONV_N(CIEUVW_TO_CIE1960UCS, from, to, from->cieuvw.u0, from->cieuvw.v0);
		return;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CONV_0(CIEXYZ_TO_CIE1960UCS, &tmp, to);
		return;
	}
}


static void
to_cieuvw(const libcolour_colour_t *restrict from, libcolour_cieuvw_t *restrict to)
{
	libcolour_colour_t tmp = *from;
	switch (from->model) {
	case LIBCOLOUR_CIEUVW:
		CONV_N(CIEUVW_TO_CIEUVW, from, to, from->cieuvw.u0, from->cieuvw.v0, to->u0, to->v0);
		break;
	default:
		tmp.model = LIBCOLOUR_CIE1960UCS;
		to_cie1960ucs(from, &tmp.cie1960ucs);
		/* fall through */
	case LIBCOLOUR_CIE1960UCS:
		CONV_N(CIE1960UCS_TO_CIEUVW, &tmp, to, to->u0, to->v0);
		break;
	}
}


int
libcolour_convert(const libcolour_colour_t *restrict from, libcolour_colour_t *restrict to)
{
#define X(C, T, N) 1 +
	if (from->model < 0 || from->model > LIST_MODELS(X) 0) {
#undef X
		errno = EINVAL;
		return -1;
	}
	switch (to->model) {
#define X(C, T, N) case C: to_##N(from, &to->N); break;
		LIST_MODELS(X)
#undef X
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}
