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



static void
ciexyz_to_rgb(const libcolour_ciexyz_t *restrict from, libcolour_rgb_t *restrict to)
{
	CIEXYZ_TO_RGB(from->X, from->Y, from->Z, to->R, to->G, to->B, to->Minv);
}


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
	case LIBCOLOUR_CIEXYZ:
		ciexyz_to_rgb(&from->ciexyz, to);
		break;
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
		ciexyz_to_rgb(&tmp, to);
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
ciexyz_to_srgb(const libcolour_ciexyz_t *restrict from, libcolour_srgb_t *restrict to)
{
	CIEXYZ_TO_SRGB(from->X, from->Y, from->Z, to->R, to->G, to->B);
}

static void
srgb_to_srgb(const libcolour_srgb_t *restrict from, libcolour_srgb_t *restrict to)
{
	if (from->with_transfer == to->with_transfer) {
		*to = *from;
	} else if (to->with_transfer) {
		to->R = srgb_encode(from->R);
		to->G = srgb_encode(from->G);
		to->B = srgb_encode(from->B);
	} else {
		to->R = srgb_decode(from->R);
		to->G = srgb_decode(from->G);
		to->B = srgb_decode(from->B);
	}
}

static void
yiq_to_srgb(const libcolour_yiq_t *restrict from, libcolour_srgb_t *restrict to)
{
	YIQ_TO_SRGB(from->Y, from->I, from->Q, to->R, to->G, to->B);
}

static void
ydbdr_to_srgb(const libcolour_ydbdr_t *restrict from, libcolour_srgb_t *restrict to)
{
	YDBDR_TO_SRGB(from->Y, from->Db, from->Dr, to->R, to->G, to->B);
}

static void
ypbpr_to_srgb(const libcolour_ypbpr_t *restrict from, libcolour_srgb_t *restrict to)
{
	YPBPR_TO_SRGB(from->Y, from->Pb, from->Pr, to->R, to->G, to->B);
}

static void
yuv_to_srgb(const libcolour_yuv_t *restrict from, libcolour_srgb_t *restrict to)
{
	YUV_TO_SRGB(from->Y, from->U, from->V, to->R, to->G, to->B);
}

static void
ycgco_to_srgb(const libcolour_ycgco_t *restrict from, libcolour_srgb_t *restrict to)
{
	YCGCO_TO_SRGB(from->Y, from->Cg, from->Co, to->R, to->G, to->B);
}

static void
other_to_srgb(const libcolour_colour_t *restrict from, libcolour_srgb_t *restrict to)
{
	libcolour_ciexyz_t tmp;
	tmp.model = LIBCOLOUR_CIEXYZ;
	to_ciexyz(from, &tmp);
	ciexyz_to_srgb(&tmp, to);
}

static void
to_srgb(const libcolour_colour_t *restrict from, libcolour_srgb_t *restrict to)
{
	libcolour_srgb_t tmp;
	switch (from->model) {
	case LIBCOLOUR_CIEXYZ:
		ciexyz_to_srgb(&from->ciexyz, to);
		break;
	case LIBCOLOUR_SRGB:
		srgb_to_srgb(&from->srgb, to);
		return;
	case LIBCOLOUR_YIQ:
		yiq_to_srgb(&from->yiq, to);
		break;
	case LIBCOLOUR_YDBDR:
		ydbdr_to_srgb(&from->ydbdr, to);
		break;
	case LIBCOLOUR_YPBPR:
		ypbpr_to_srgb(&from->ypbpr, to);
		break;
	case LIBCOLOUR_YUV:
		yuv_to_srgb(&from->yuv, to);
		break;
	case LIBCOLOUR_YCGCO:
		ycgco_to_srgb(&from->ycgco, to);
		break;
	default:
		other_to_srgb(from, to);
		break;
	}
	if (to->with_transfer) {
		tmp = *to;
		tmp.with_transfer = 0;
		srgb_to_srgb(&tmp, to);
	}
}


static void
ciexyz_to_ciexyy(const libcolour_ciexyz_t *restrict from, libcolour_ciexyy_t *restrict to)
{
	CIEXYZ_TO_CIEXYY(from->X, from->Y, from->Z, to->x, to->y, to->Y);
}

static void
other_to_ciexyy(const libcolour_colour_t *restrict from, libcolour_ciexyy_t *restrict to)
{
	libcolour_ciexyz_t tmp;
	tmp.model = LIBCOLOUR_CIEXYZ;
	to_ciexyz(from, &tmp);
	ciexyz_to_ciexyy(&tmp, to);
}

static void
srgb_to_ciexyy(const libcolour_srgb_t *restrict from, libcolour_ciexyy_t *restrict to)
{
	libcolour_srgb_t tmp;
	tmp.model = LIBCOLOUR_SRGB;
	if (from->with_transfer) {
		tmp.with_transfer = 0;
		to_srgb((const libcolour_colour_t*)from, &tmp);
	} else {
		tmp = *from;
	}
	if (tmp.R == 0 && tmp.G == 0 && tmp.B == 0) {
		to->x = D(0.31272660439158);
		to->y = D(0.32902315240275);
		to->Y = 0;
	} else {
		other_to_ciexyy((const libcolour_colour_t*)&tmp, to);
	}
}

static void
to_ciexyy(const libcolour_colour_t *restrict from, libcolour_ciexyy_t *restrict to)
{
	switch (from->model) {
	case LIBCOLOUR_CIEXYY:
		*to = from->ciexyy;
		return;
	case LIBCOLOUR_CIEXYZ:
		ciexyz_to_ciexyy(&from->ciexyz, to);
		return;
	case LIBCOLOUR_SRGB:
		srgb_to_ciexyy(&from->srgb, to);
		return;
	default:
		other_to_ciexyy(from, to);
		return;
	}
}


static void
rgb_to_ciexyz(const libcolour_rgb_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	libcolour_rgb_t tmp;
	TYPE R, G, B;
	if (from->with_transfer) {
		tmp = *from;
		tmp.with_transfer = 0;
		to_rgb((const libcolour_colour_t *)from, &tmp);
		R = tmp.R, G = tmp.G, B = tmp.B;
	} else {
		R = from->R, G = from->G, B = from->B;
	}
	RGB_TO_CIEXYZ(R, G, B, to->X, to->Y, to->Z, from->M);
}

static void
srgb_to_ciexyz(const libcolour_srgb_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	libcolour_srgb_t tmp;
	TYPE R, G, B;
	if (from->with_transfer) {
		tmp.model = LIBCOLOUR_SRGB;
		tmp.with_transfer = 0;
		to_srgb((const libcolour_colour_t*)from, &tmp);
		R = tmp.R, G = tmp.G, B = tmp.B;
	} else {
		R = from->R, G = from->G, B = from->B;
	}
	SRGB_TO_CIEXYZ(R, G, B, to->X, to->Y, to->Z);
}

static void
ciexyy_to_ciexyz(const libcolour_ciexyy_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	CIEXYY_TO_CIEXYZ(from->x, from->y, from->Y, to->X, to->Y, to->Z);
}

static void
cielab_to_ciexyz(const libcolour_cielab_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	CIELAB_TO_CIEXYZ(from->L, from->a, from->b, to->X, to->Y, to->Z);
}

static void
cieluv_to_ciexyz(const libcolour_cieluv_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	CIELUV_TO_CIEXYZ(from->L, from->u, from->v, to->X, to->Y, to->Z,
			 from->white.X, from->white.Y, from->white.Z);
}

static void
cielchuv_to_ciexyz(const libcolour_cielchuv_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	libcolour_cieluv_t tmp;
	tmp.model = LIBCOLOUR_CIELUV;
	tmp.white = from->white;
	to_cieluv((const libcolour_colour_t*)from, &tmp);
	cieluv_to_ciexyz(&tmp, to);
}

static void
yiq_to_ciexyz(const libcolour_yiq_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	YIQ_TO_CIEXYZ(from->Y, from->I, from->Q, to->X, to->Y, to->Z);
}

static void
ydbdr_to_ciexyz(const libcolour_ydbdr_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	YDBDR_TO_CIEXYZ(from->Y, from->Db, from->Dr, to->X, to->Y, to->Z);
}

static void
yuv_to_ciexyz(const libcolour_yuv_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	YUV_TO_CIEXYZ(from->Y, from->U, from->V, to->X, to->Y, to->Z);
}

static void
ypbpr_to_ciexyz(const libcolour_ypbpr_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	YPBPR_TO_CIEXYZ(from->Y, from->Pb, from->Pr, to->X, to->Y, to->Z);
}

static void
ycgco_to_ciexyz(const libcolour_ycgco_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	YCGCO_TO_CIEXYZ(from->Y, from->Cg, from->Co, to->X, to->Y, to->Z);
}

static void
cie1960ucs_to_ciexyz(const libcolour_cie1960ucs_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	CIE1960UCS_TO_CIEXYZ(from->u, from->v, from->Y, to->X, to->Y, to->Z);
}

static void
cieuvw_to_ciexyz(const libcolour_cieuvw_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	libcolour_cie1960ucs_t tmp;
	tmp.model = LIBCOLOUR_CIE1960UCS;
	to_cie1960ucs((const libcolour_colour_t *)from, &tmp);
	cie1960ucs_to_ciexyz(&tmp, to);
}

static void
other_to_ciexyz(const libcolour_colour_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	libcolour_srgb_t tmp;
	tmp.model = LIBCOLOUR_SRGB;
	tmp.with_transfer = 0;
	to_srgb(from, &tmp);
	srgb_to_ciexyz(&tmp, to);
}

static void to_ciexyz(const libcolour_colour_t *restrict from, libcolour_ciexyz_t *restrict to)
{
	switch (from->model) {
	case LIBCOLOUR_RGB:
		rgb_to_ciexyz(&from->rgb, to);
		break;
	case LIBCOLOUR_SRGB:
		srgb_to_ciexyz(&from->srgb, to);
		break;
	case LIBCOLOUR_CIEXYY:
		ciexyy_to_ciexyz(&from->ciexyy, to);
		break;
	case LIBCOLOUR_CIEXYZ:
		*to = from->ciexyz;
		break;
	case LIBCOLOUR_CIELAB:
		cielab_to_ciexyz(&from->cielab, to);
		break;
	case LIBCOLOUR_CIELUV:
		cieluv_to_ciexyz(&from->cieluv, to);
		break;
	case LIBCOLOUR_CIELCHUV:
		cielchuv_to_ciexyz(&from->cielchuv, to);
		break;
	case LIBCOLOUR_YIQ:
		yiq_to_ciexyz(&from->yiq, to);
		break;
	case LIBCOLOUR_YDBDR:
		ydbdr_to_ciexyz(&from->ydbdr, to);
		break;
	case LIBCOLOUR_YUV:
		yuv_to_ciexyz(&from->yuv, to);
		break;
	case LIBCOLOUR_YPBPR:
		ypbpr_to_ciexyz(&from->ypbpr, to);
		break;
	case LIBCOLOUR_YCGCO:
		ycgco_to_ciexyz(&from->ycgco, to);
		break;
	case LIBCOLOUR_CIE1960UCS:
		cie1960ucs_to_ciexyz(&from->cie1960ucs, to);
		break;
	case LIBCOLOUR_CIEUVW:
		cieuvw_to_ciexyz(&from->cieuvw, to);
		break;
	default:
		other_to_ciexyz(from, to);
		break;
	}
}


static void
ciexyz_to_cielab(const libcolour_ciexyz_t *restrict from, libcolour_cielab_t *restrict to)
{
	CIEXYZ_TO_CIELAB(from->X, from->Y, from->Z, to->L, to->a, to->b);
}

static void
other_to_cielab(const libcolour_colour_t *restrict from, libcolour_cielab_t *restrict to)
{
	libcolour_ciexyz_t tmp;
	tmp.model = LIBCOLOUR_CIEXYZ;
	to_ciexyz(from, &tmp);
	ciexyz_to_cielab(&tmp, to);
}

static void
to_cielab(const libcolour_colour_t *restrict from, libcolour_cielab_t *restrict to)
{
	switch (from->model) {
	case LIBCOLOUR_CIEXYZ:
		ciexyz_to_cielab(&from->ciexyz, to);
		return;
	case LIBCOLOUR_CIELAB:
		*to = from->cielab;
		return;
	default:
		other_to_cielab(from, to);
		return;
	}
}


static void
ciexyz_to_cieluv(const libcolour_ciexyz_t *restrict from, libcolour_cieluv_t *restrict to)
{
	CIEXYZ_TO_CIELUV(from->X, from->Y, from->Z, to->L, to->u, to->v,
			 to->white.X, to->white.Y, to->white.Z);
}

static void
cielchuv_to_cieluv(const libcolour_cielchuv_t *restrict from, libcolour_cieluv_t *restrict to)
{
	libcolour_ciexyz_t tmp;
	libcolour_cielchuv_t tmp2;
	TYPE L, C, h;
	if (to->white.X != from->white.X || to->white.Y != from->white.Y || to->white.Z != from->white.Z) {
		tmp.model = LIBCOLOUR_CIEXYZ;
		tmp2.model = LIBCOLOUR_CIELCHUV;
		tmp2.white = to->white;
		tmp2.one_revolution = PI2;
		to_ciexyz((const libcolour_colour_t*)from, &tmp);
		to_cielchuv((const libcolour_colour_t*)&tmp, &tmp2);
		L = tmp2.L, C = tmp2.C, h = tmp2.h;
	} else {
		L = from->L, C = from->C, h = from->h * PI2 / from->one_revolution;
	}
	CIELCHUV_TO_CIELUV(L, C, h, to->L, to->u, to->v);
}

static void
other_to_cieluv(const libcolour_colour_t *restrict from, libcolour_cieluv_t *restrict to)
{
	libcolour_ciexyz_t tmp;
	tmp.model = LIBCOLOUR_CIEXYZ;
	to_ciexyz(from, &tmp);
	ciexyz_to_cieluv(&tmp, to);
}

static void
to_cieluv(const libcolour_colour_t *restrict from, libcolour_cieluv_t *restrict to)
{
	switch (from->model) {
	case LIBCOLOUR_CIEXYZ:
		ciexyz_to_cieluv(&from->ciexyz, to);
		break;
	case LIBCOLOUR_CIELCHUV:
		cielchuv_to_cieluv(&from->cielchuv, to);
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
		other_to_cieluv(from, to);
		break;
	}
}


static void
cieluv_to_cielchuv(const libcolour_cieluv_t *restrict from, libcolour_cielchuv_t *restrict to)
{
	libcolour_cieluv_t tmp;
	TYPE L, u, v;
	if (to->white.X != from->white.X || to->white.Y != from->white.Y || to->white.Z != from->white.Z) {
		tmp.model = LIBCOLOUR_CIELUV;
		tmp.white = to->white;
		to_cieluv((const libcolour_colour_t *)from, &tmp);
		L = tmp.L, u = tmp.u, v = tmp.v;
	} else {
		L = from->L, u = from->u, v = from->v;
	}
	CIELUV_TO_CIELCHUV(L, u, v, to->L, to->C, to->h, to->one_revolution);
}

static void
other_to_cielchuv(const libcolour_colour_t *restrict from, libcolour_cielchuv_t *restrict to)
{
	libcolour_cieluv_t tmp;
	tmp.model = LIBCOLOUR_CIELUV;
	tmp.white = to->white;
	to_cieluv(from, &tmp);
	cieluv_to_cielchuv(&tmp, to);
}

static void
to_cielchuv(const libcolour_colour_t *restrict from, libcolour_cielchuv_t *restrict to)
{
	TYPE one_revolution;
	switch (from->model) {
	case LIBCOLOUR_CIELUV:
		cieluv_to_cielchuv(&from->cieluv, to);
		break;
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
		other_to_cielchuv(from, to);
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
		SRGB_TO_YIQ(tmp.srgb.R, tmp.srgb.G, tmp.srgb.B, to->Y, to->I, to->Q);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CIEXYZ_TO_YIQ(tmp.ciexyz.X, tmp.ciexyz.Y, tmp.ciexyz.Z, to->Y, to->I, to->Q);
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
		SRGB_TO_YDBDR(tmp.srgb.R, tmp.srgb.G, tmp.srgb.B, to->Y, to->Db, to->Dr);
		break;
	case LIBCOLOUR_YUV:
		YUV_TO_YDBDR(tmp.yuv.Y, tmp.yuv.U, tmp.yuv.V, to->Y, to->Db, to->Dr);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CIEXYZ_TO_YDBDR(tmp.ciexyz.X, tmp.ciexyz.Y, tmp.ciexyz.Z, to->Y, to->Db, to->Dr);
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
		YDBDR_TO_YUV(from->ydbdr.Y, from->ydbdr.Db, from->ydbdr.Dr, to->Y, to->U, to->V);
		break;
	case LIBCOLOUR_SRGB:
		if (tmp.srgb.with_transfer) {
			tmp.srgb.with_transfer = 0;
			to_srgb(from, &tmp.srgb);
		}
		SRGB_TO_YUV(tmp.srgb.R, tmp.srgb.G, tmp.srgb.B, to->Y, to->U, to->V);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CIEXYZ_TO_YUV(tmp.ciexyz.X, tmp.ciexyz.Y, tmp.ciexyz.Z, to->Y, to->U, to->V);
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
		SRGB_TO_YPBPR(tmp.srgb.R, tmp.srgb.G, tmp.srgb.B, to->Y, to->Pb, to->Pr);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CIEXYZ_TO_YPBPR(tmp.ciexyz.X, tmp.ciexyz.Y, tmp.ciexyz.Z, to->Y, to->Pb, to->Pr);
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
		SRGB_TO_YCGCO(tmp.srgb.R, tmp.srgb.G, tmp.srgb.B, to->Y, to->Cg, to->Co);
		break;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CIEXYZ_TO_YCGCO(tmp.ciexyz.X, tmp.ciexyz.Y, tmp.ciexyz.Z, to->Y, to->Cg, to->Co);
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
		CIEUVW_TO_CIE1960UCS(from->cieuvw.U, from->cieuvw.V, from->cieuvw.W,
				     to->u, to->v, to->Y, from->cieuvw.u0, from->cieuvw.v0);
		return;
	default:
		tmp.model = LIBCOLOUR_CIEXYZ;
		to_ciexyz(from, &tmp.ciexyz);
		/* fall through */
	case LIBCOLOUR_CIEXYZ:
		CIEXYZ_TO_CIE1960UCS(tmp.ciexyz.X, tmp.ciexyz.Y, tmp.ciexyz.Z, to->u, to->v, to->Y);
		return;
	}
}


static void
to_cieuvw(const libcolour_colour_t *restrict from, libcolour_cieuvw_t *restrict to)
{
	libcolour_colour_t tmp = *from;
	switch (from->model) {
	case LIBCOLOUR_CIEUVW:
		CIEUVW_TO_CIEUVW(from->cieuvw.U, from->cieuvw.V, from->cieuvw.W, to->U, to->V, to->W,
				 from->cieuvw.u0, from->cieuvw.v0, to->u0, to->v0);
		break;
	default:
		tmp.model = LIBCOLOUR_CIE1960UCS;
		to_cie1960ucs(from, &tmp.cie1960ucs);
		/* fall through */
	case LIBCOLOUR_CIE1960UCS:
		CIE1960UCS_TO_CIEUVW(tmp.cie1960ucs.u, tmp.cie1960ucs.v, tmp.cie1960ucs.Y,
				     to->U, to->V, to->W, to->u0, to->v0);
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
	case LIBCOLOUR_RGB:
		to_rgb(from, &to->rgb);
		break;
	case LIBCOLOUR_SRGB:
		to_srgb(from, &to->srgb);
		break;
	case LIBCOLOUR_CIEXYY:
		to_ciexyy(from, &to->ciexyy);
		break;
	case LIBCOLOUR_CIEXYZ:
		to_ciexyz(from, &to->ciexyz);
		break;
	case LIBCOLOUR_CIELAB:
		to_cielab(from, &to->cielab);
		break;
	case LIBCOLOUR_CIELUV:
		to_cieluv(from, &to->cieluv);
		break;
	case LIBCOLOUR_CIELCHUV:
		to_cielchuv(from, &to->cielchuv);
		break;
	case LIBCOLOUR_YIQ:
		to_yiq(from, &to->yiq);
		break;
	case LIBCOLOUR_YDBDR:
		to_ydbdr(from, &to->ydbdr);
		break;
	case LIBCOLOUR_YUV:
		to_yuv(from, &to->yuv);
		break;
	case LIBCOLOUR_YPBPR:
		to_ypbpr(from, &to->ypbpr);
		break;
	case LIBCOLOUR_YCGCO:
		to_ycgco(from, &to->ycgco);
		break;
	case LIBCOLOUR_CIE1960UCS:
		to_cie1960ucs(from, &to->cie1960ucs);
		break;
	case LIBCOLOUR_CIEUVW:
		to_cieuvw(from, &to->cieuvw);
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}
