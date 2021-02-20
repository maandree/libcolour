/* See LICENSE file for copyright and license details. */
#include "libcolour.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#undef libcolour_unmarshal

#define libcolour_colour_t  libcolour_colour_lf_t
#define libcolour_rgb_t     libcolour_rgb_lf_t
#define libcolour_srgb_t    libcolour_srgb_lf_t
#define libcolour_ciexyy_t  libcolour_ciexyy_lf_t
#define libcolour_unmarshal libcolour_unmarshal_lf



#define SLOPE transfer.regular.slope
#define TRANSITIONINV transfer.regular.transitioninv
#define TRANSITION transfer.regular.transition
#define GAMMA transfer.regular.gamma
#define OFFSET transfer.regular.offset
#define TO_ENCODED_RED transfer.custom.to_encoded_red
#define TO_DECODED_RED transfer.custom.to_decoded_red
#define TO_ENCODED_GREEN transfer.custom.to_encoded_green
#define TO_DECODED_GREEN transfer.custom.to_decoded_green
#define TO_ENCODED_BLUE transfer.custom.to_encoded_blue
#define TO_DECODED_BLUE transfer.custom.to_decoded_blue



#ifndef SKIP_2CONVERT
static int
test_2convert(libcolour_colour_t *c1, libcolour_colour_t *c2, libcolour_colour_t *c3)
{
	double ch1, ch2, ch3;
	if (libcolour_convert(c1, c2))
		return -1;
	if (libcolour_convert(c2, c3))
		return -1;
	if (!(isnan(c2->srgb.R) || isnan(c2->srgb.G) || isnan(c2->srgb.B)))
		if ((isnan(c3->srgb.R) || isnan(c3->srgb.G) || isnan(c3->srgb.B)))
			fprintf(stderr, "(%lf %lf %lf)[%i] -> (%lf %lf %lf)[%i] -> (%lf %lf %lf)[%i]\n",
				c1->srgb.R, c1->srgb.G, c1->srgb.B, c1->model,
				c2->srgb.R, c2->srgb.G, c2->srgb.B, c2->model,
				c3->srgb.R, c3->srgb.G, c3->srgb.B, c3->model);
	if ((c1->model == LIBCOLOUR_CIELCHUV) &&
	    !isnan(c1->cielchuv.h) && !isnan(c3->cielchuv.h) &&
	    !isinf(c1->cielchuv.h) && !isinf(c3->cielchuv.h) &&
	    (c3->cielchuv.h / c3->cielchuv.one_revolution > 0.99999))
		c3->cielchuv.h = 0;
	ch1 = c1->srgb.R - c3->srgb.R;
	ch2 = c1->srgb.G - c3->srgb.G;
	ch3 = c1->srgb.B - c3->srgb.B;
	ch1 *= ch1;
	ch2 *= ch2;
	ch3 *= ch3;
	switch (c1->model) {
	case LIBCOLOUR_CIEXYY:
		if (c1->ciexyy.Y == 0)
			ch1 = ch2 = 0;
		if (c1->ciexyy.y == 0)
			ch1 = ch2 = ch3 = 0;
		break;
	case LIBCOLOUR_CIEXYZ:
		if (c1->ciexyz.Y == 0)
			ch1 = ch3 = 0;
		break;
	case LIBCOLOUR_CIE1960UCS:
		if (c1->cie1960ucs.v == 0)
			ch1 = ch2 = ch3 = 0;
		if (c1->cie1960ucs.Y == 0)
			ch1 = ch2 = 0;
		break;
	case LIBCOLOUR_CIEUVW:
		if (c1->cieuvw.W == 0)
			ch1 = ch2 = 0;
		break;
	case LIBCOLOUR_CIELCHUV:
		if (c1->cielchuv.C == 0)
			ch1 = ch3 = 0;
		break;
	case LIBCOLOUR_CIELAB:
		if (c1->cielchuv.L == 0)
			ch2 = ch3 = 0;
		break;
	default:
		break;
	}
	if (ch1 > 0.000001 ||
	    ch2 > 0.000001 ||
	    ch3 > 0.000001)
		return 0;
	return 1;
}


static int
test_2convert_11(libcolour_colour_t *c1, libcolour_model_t model)
{
	libcolour_colour_t c2, c3;
	int r1, r2;
	c3 = *c1;
	c2.model = model;
	switch (model) {
	case LIBCOLOUR_RGB:
		c2.rgb.with_transfer = 0;
		if (libcolour_get_rgb_colour_space(&c2.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB) < 0)
			return -1;
		return test_2convert(c1, &c2, &c3);
	case LIBCOLOUR_SRGB:
		c2.srgb.with_transfer = 0;
		if (r1 = test_2convert(c1, &c2, &c3), r1 < 0)
			return -1;
		c2.srgb.with_transfer = 1;
		if (r2 = test_2convert(c1, &c2, &c3), r2 < 0)
			return -1;
		return r1 & r2;
	case LIBCOLOUR_CIELCHUV:
		c2.cielchuv.one_revolution = 360.;
		/* fall through */
	case LIBCOLOUR_CIELUV:
		c2.cieluv.white.model = LIBCOLOUR_CIEXYZ;
		c2.cieluv.white.X = 1.0294;
		c2.cieluv.white.Y = 1;
		c2.cieluv.white.Z = 0.9118;
		return test_2convert(c1, &c2, &c3);
	case LIBCOLOUR_CIEUVW:
		c2.cieuvw.u0 = 0.37;
		c2.cieuvw.v0 = 0.30;
		return test_2convert(c1, &c2, &c3);
	default:
		return test_2convert(c1, &c2, &c3);
	}
}


static int
test_2convert_1n(libcolour_model_t model, const char *model_name, double ch1, double ch2, double ch3)
{
	libcolour_colour_t c1;
	int run, r, rc = 1;

	c1.model = model;
	switch (model) {
	case LIBCOLOUR_CIELAB:
		c1.cielab.L = ch1 * 100, c1.cielab.a = ch2 * 100, c1.cielab.b = ch3 * 100;
		break;
	case LIBCOLOUR_CIELCHUV:
		if (ch3 > 0.9999)
			return 1;
		/* fall through */
	case LIBCOLOUR_CIELUV:
		if (ch1 <= 0.00001 && (ch2 > 0.00001 || ch3 > 0.00001))
			return 1;
		/* fall through */
	default:
		c1.srgb.R = ch1, c1.srgb.G = ch2, c1.srgb.B = ch3;
		break;
	}

	for (run = 0;; run++) {
		switch (model) {
		case LIBCOLOUR_RGB:
			if (run == 2)
				return rc;
			c1.rgb.with_transfer = run;
			if (libcolour_get_rgb_colour_space(&c1.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB) < 0)
				return -1;
			break;
		case LIBCOLOUR_SRGB:
			if (run == 2)
				return rc;
			c1.srgb.with_transfer = run;
			break;
		case LIBCOLOUR_CIELCHUV:
			c1.cielchuv.one_revolution = run < 2 ? 360. : 2 * 3.14159265358979323846;
			c1.cielchuv.h *= c1.cielchuv.one_revolution;
			/* fall through */
		case LIBCOLOUR_CIELUV:
			c1.cieluv.white.model = LIBCOLOUR_CIEXYZ;
			if (run == 0) {
				c1.cieluv.white.X = 1.0294;
				c1.cieluv.white.Y = 1;
				c1.cieluv.white.Z = 0.9118;
			} else if (run == 1) {
				c1.cieluv.white.X = 1.03;
				c1.cieluv.white.Y = 0.8;
				c1.cieluv.white.Z = 0.92;
			} else if (run == 2 && model == LIBCOLOUR_CIELCHUV) {
				c1.cieluv.white.X = 1.0294;
				c1.cieluv.white.Y = 1;
				c1.cieluv.white.Z = 0.9118;
			} else if (run == 3 && model == LIBCOLOUR_CIELCHUV) {
				c1.cieluv.white.X = 1.03; 
				c1.cieluv.white.Y = 0.8;
				c1.cieluv.white.Z = 0.92;
			} else {
				return rc;
			}
			break;
		case LIBCOLOUR_CIEUVW:
			if (run == 0) {
				c1.cieuvw.u0 = 0.37;
				c1.cieuvw.v0 = 0.30;
			} else if (run == 1) {
				c1.cieuvw.u0 = 0.47;
				c1.cieuvw.v0 = 0.31;
			} else {
				return rc;
			}
			break;
		default:
			if (run == 1)
				return rc;
			break;
		}

#define X(ENUM, T, N)\
		r = test_2convert_11(&c1, ENUM);\
		if (r < 0)\
			return -1;\
		if (!r)\
			printf("%s -> %s -> %s failed at run %i with (%lf, %lf, %lf)\n",\
			       model_name, #ENUM, model_name, run, ch1, ch2, ch3), rc = 0;
		LIBCOLOUR_LIST_MODELS(X,);
#undef X
	}
}


static int
test_2convert_nm(double ch1, double ch2, double ch3)
{
	int r, rc = 1;

#define X(ENUM, T, N)\
	r = test_2convert_1n(ENUM, #ENUM, ch1, ch2, ch3);\
	if (r < 0)\
		return -1;\
	if (!r)\
		rc = 0;
	LIBCOLOUR_LIST_MODELS(X,);
#undef X

	return rc;
}


static int
test_2convert_nm_all(void)
{
#define N 11
	int r, rc = 1;
	double ch1, ch2, ch3;
	int i, j, k;

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (k = 0; k < N; k++) {
				ch1 = ((i + N / 2) % N) / (N - 1);
				ch2 = ((j + N / 2) % N) / (N - 1);
				ch3 = ((k + N / 2) % N) / (N - 1);

				if (ch1 > 0.999) ch1 = 1;
				if (ch2 > 0.999) ch2 = 1;
				if (ch3 > 0.999) ch3 = 1;

				r = test_2convert_nm(ch1, ch2, ch3);
				if (r < 0)
					return -1;
				if (!r)
					rc = 0;
			}
		}
	}

	return rc;
#undef N
}
#endif


static inline int
ftest(double a, double b, double l)
{
	return a - l <= b && b <= a + l;
}


#define test_rgb(COLOUR_SPACE, ...) test_rgb_(COLOUR_SPACE, #COLOUR_SPACE, __VA_ARGS__)
static int
test_rgb_(enum libcolour_rgb_colour_space colour_space, const char *name,
	  double r, double g, double b, double x, double y, double z,
	  double xx, double yy, double zz)
{
	libcolour_colour_t c1, c2;

	c1.model = LIBCOLOUR_RGB;
	c2.model = LIBCOLOUR_CIEXYZ;
	c1.rgb.with_transfer = 0;
	c1.rgb.R = r, c1.rgb.G = g, c1.rgb.B = b;
	if (libcolour_get_rgb_colour_space(&c1.rgb, colour_space) ||
	    libcolour_convert(&c1, &c2)) {
		printf("%s failed without gamma\n", name);
		return 0;
	} else if (!ftest(c2.ciexyz.X, x, 0.0001) ||
		   !ftest(c2.ciexyz.Y, y, 0.0001) ||
		   !ftest(c2.ciexyz.Z, z, 0.0001)) {
		printf("%s failed without gamma\n", name);
		return 0;
	}

	c1.model = LIBCOLOUR_RGB;
	c2.model = LIBCOLOUR_CIEXYZ;
	c1.rgb.with_transfer = 1;
	c1.rgb.R = r, c1.rgb.G = g, c1.rgb.B = b;
	if (libcolour_get_rgb_colour_space(&c1.rgb, colour_space) ||
	    libcolour_convert(&c1, &c2)) {
		printf("%s failed with gamma\n", name);
		return 0;
	} else if (!ftest(c2.ciexyz.X, xx, 0.0001) ||
		   !ftest(c2.ciexyz.Y, yy, 0.0001) ||
		   !ftest(c2.ciexyz.Z, zz, 0.0001)) {
		printf("%s failed with gamma\n", name);
		return 0;
	}

	if (libcolour_convert(&c2, &c1) ||
	    !ftest(c1.rgb.R, r, 0.0000001) ||
	    !ftest(c1.rgb.G, g, 0.0000001) ||
	    !ftest(c1.rgb.B, b, 0.0000001)) {
		printf("%s failed, encode\n", name);
		return 0;
	}

	return 1;
}


static int
test_1convert(libcolour_colour_t *c1, libcolour_model_t model, const char *name1,
	      const char *name2, double ch1, double ch2, double ch3, double d1,
	      double d2, double d3)
{
	libcolour_colour_t c2, c3;
	int rc = 1;
	c3 = *c1;
	c2.model = model;
	switch (model) {
	case LIBCOLOUR_CIELCHUV:
		c2.cielchuv.one_revolution = 360;
		/* fall-through */
	case LIBCOLOUR_CIELUV:
		c2.cieluv.white.X = 0.9504705586542831;
		c2.cieluv.white.Y = 1;
		c2.cieluv.white.Z = 1.0888287363958837;
		break;
	default:
		break;
	}
	if (libcolour_convert(c1, &c2))
		return -1;
	if (!ftest(c2.srgb.R, ch1, d1) ||
	    !ftest(c2.srgb.G, ch2, d2) ||
	    !ftest(c2.srgb.B, ch3, d3)) {
		printf("%s -> %s failed\n", name1, name2), rc = 0;
		rc = 0;
	}
	if (libcolour_convert(&c2, &c3))
		return -1;
	if (!ftest(c3.srgb.R, c1->srgb.R, 0.00000001) ||
	    !ftest(c3.srgb.G, c1->srgb.G, 0.00000001) ||
	    !ftest(c3.srgb.B, c1->srgb.B, 0.00000001)) {
		printf("%s -> %s failed\n", name2, name1), rc = 0;
		rc = 0;
	}
	return rc;
}


static int
test_en_masse(libcolour_colour_t *c1, libcolour_colour_t *c2, libcolour_colour_t *c3, int mode)
{
	double in1[8], in2[2], in3[2], in4[2], out1[8], out2[2], out3[2], out4[2];

#define SET(R1, G1, B1, A1, R2, G2, B2, A2)\
	((R1) = c1->srgb.R, (R2) = c2->srgb.R,\
	 (G1) = c1->srgb.G, (G2) = c2->srgb.G,\
	 (B1) = c1->srgb.B, (B2) = c2->srgb.B,\
	 (A1) = 0.4, (A2) = 0.6)

#define CHECK(C, RR, GG, BB, AA, A)\
	do {\
		if (libcolour_convert((C), c3))\
			printf("libcolour_convert failed\n");\
		if ((mode & 3) == LIBCOLOUR_CONVERT_EN_MASSE_NO_ALPHA)\
			(AA) = (A);\
		if ((mode & 3) == LIBCOLOUR_CONVERT_EN_MASSE_SEPARATED)\
			(AA) = (A);\
		if ((RR) != c3->srgb.R || (GG) != c3->srgb.G || (BB) != c3->srgb.B || (AA) != (A))\
			return printf("libcolour_convert_en_masse(mode=%i) failed, "\
				      "got (%lf, %lf, %lf, %lf), "\
				      "expected (%lf, %lf, %lf, %lf)\n",\
				      mode, (RR), (GG), (BB), (AA),\
				      c3->srgb.R, c3->srgb.G, c3->srgb.B, (A)), -1;\
	} while (0)

#define TEST(R1, G1, B1, A1, R2, G2, B2, A2, ...)\
	do {\
		SET(in##R1, in##G1, in##B1, in##A1, in##R2, in##G2, in##B2, in##A2);\
		if (libcolour_convert_en_masse(c1, c3, mode, 2, __VA_ARGS__))\
			return printf("libcolour_convert_en_masse(mode=%i) failed", mode), -2;\
		if (mode & LIBCOLOUR_CONVERT_EN_MASSE_NO_OVERRIDE) {\
			CHECK(c1, out##R1, out##G1, out##B1, out##A1, 0.4);\
			CHECK(c2, out##R2, out##G2, out##B2, out##A2, 0.6);\
		} else {\
			CHECK(c1, in##R1, in##G1, in##B1, in##A1, 0.4);\
			CHECK(c2, in##R2, in##G2, in##B2, in##A2, 0.6);\
		}\
	} while (0)

	switch (mode & 3) {
	case LIBCOLOUR_CONVERT_EN_MASSE_NO_ALPHA:
		TEST(1[0], 1[1], 1[2], 1[6], 1[3], 1[4], 1[5], 1[7], in1, out1);
		break;
	case LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_FIRST:
		TEST(1[1], 1[2], 1[3], 1[0], 1[5], 1[6], 1[7], 1[4], in1, out1);
		break;
	case LIBCOLOUR_CONVERT_EN_MASSE_ALPHA_LAST:
		TEST(1[0], 1[1], 1[2], 1[3], 1[4], 1[5], 1[6], 1[7], in1, out1);
		break;
	case LIBCOLOUR_CONVERT_EN_MASSE_SEPARATED:
		TEST(1[0], 2[0], 3[0], 4[0], 1[1], 2[1], 3[1], 4[1], in1, in2, in3, out1, out2, out3);
		break;
	default:
		abort();
	}

#undef SET
#undef CHECK
#undef TEST

	return 0;
}


static int
test_partitioned_en_masse(void)
{
	libcolour_srgb_t from;
	libcolour_ciexyy_t to;
	double ch1[32], ch2[32], ch3[32];
	int i;

	from.model = LIBCOLOUR_SRGB;
	from.with_transfer = 0;
	to.model = LIBCOLOUR_CIEXYY;

	for (i = 0; i < 32; i++) {
		ch1[i] = (i + 1.) / 32.;
		ch2[i] = (i + 1.) / 32.;
		ch3[i] = (i + 1.) / 32.;
	}

	for (i =  3; i <  6; i++) ch1[i] = ch2[i] = ch3[i] = 0;
	for (i = 20; i < 30; i++) ch1[i] = ch2[i] = ch3[i] = 0;

	if (libcolour_convert_en_masse(&from, &to, LIBCOLOUR_CONVERT_EN_MASSE_SEPARATED, 32, ch1, ch2, ch3))
		return -1;

	for (i = 0; i < 32; i++) {
		if ((3 <= i && i < 6) || (20 <= i && i < 30))
			from.R = from.G = from.B = 0;
		else
			from.R = from.G = from.B = (i + 1.) / 32.;
		if (libcolour_convert(&from, &to))
			return -1;
		if (ch1[i] != to.x || ch2[i] != to.y || ch3[i] != to.Y) {
			printf("test_partitioned_en_masse failed at %i\n", i);
			return -1;
		}
	}

	return 0;
}


/**
 * Test libcolour
 * 
 * @return  0: All tests passed
 *          1: At least one test fail
 *          2: An error occurred
 */
int
main(int argc, char *argv[])
{
	int r, rc = 0, i;
	libcolour_colour_t c1, c2, c3, c4;
	double t1, t2;
	size_t n;
	char buf[sizeof(int) + sizeof(libcolour_rgb_t)];

#ifndef SKIP_2CONVERT
	r = test_2convert_nm_all();
	if (r < 0)
		goto fail;
	if (!r)
		rc = 1;
#else
	(void) r;
#endif

	c1.model = LIBCOLOUR_SRGB;
	c1.srgb.R = 0.02, c1.srgb.G = 0.5, c1.srgb.B = 0.9;
	c1.srgb.with_transfer = 0;
	c2.model = LIBCOLOUR_RGB;
	c2.rgb.with_transfer = 0;
	if (libcolour_get_rgb_colour_space(&c2.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB)) {
		printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 1;
		goto colour_spaces_done;
	}
	if (c2.rgb.white_r != 1 || c2.rgb.white_g != 1 || c2.rgb.white_b != 1 ||
	    c2.rgb.red.model != LIBCOLOUR_CIEXYY || c2.rgb.green.model != LIBCOLOUR_CIEXYY ||
	    c2.rgb.blue.model != LIBCOLOUR_CIEXYY || c2.rgb.white.model != LIBCOLOUR_CIEXYY ||
	    c2.rgb.colour_space != LIBCOLOUR_RGB_COLOUR_SPACE_SRGB || c2.rgb.GAMMA != 2.4 ||
	    c2.rgb.encoding_type != LIBCOLOUR_ENCODING_TYPE_REGULAR || c2.rgb.SLOPE != 12.92
	    || c2.rgb.OFFSET != 0.055 || c2.rgb.TRANSITIONINV != c2.rgb.TRANSITION * c2.rgb.SLOPE ||
	    !ftest(c2.rgb.TRANSITION, 0.00313067, 0.00000001)) {
		printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 1;
		goto colour_spaces_done;
	}
	t1 = c2.rgb.TRANSITION * c2.rgb.SLOPE;
	t2 = (1 + c2.rgb.OFFSET) * pow(c2.rgb.TRANSITION, 1 / c2.rgb.GAMMA) - c2.rgb.OFFSET;
	if (!ftest(t1, t2, 0.0000001) || c2.rgb.white.Y != 1 ||
	    c2.rgb.red.x != 0.64 || c2.rgb.red.y != 0.33 || !ftest(c2.rgb.red.Y, 0.21265, 0.00005) ||
	    c2.rgb.green.x != 0.30 || c2.rgb.green.y != 0.60 || !ftest(c2.rgb.green.Y, 0.71515, 0.00005) ||
	    c2.rgb.blue.x != 0.15 || c2.rgb.blue.y != 0.06 || !ftest(c2.rgb.blue.Y, 0.07215, 0.00005) ||
	    !ftest(c2.rgb.white.x, 0.312727, 0.000001) || !ftest(c2.rgb.white.y, 0.329023, 0.000001) ||
	    !ftest(c2.rgb.red.Y + c2.rgb.green.Y + c2.rgb.blue.Y, 1, 0.00000001)) {
		printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 1;
		goto colour_spaces_done;
	}
	if (libcolour_convert(&c1, &c2) ||
	    !ftest(c1.srgb.R, c2.rgb.R, 0.0000001) ||
	    !ftest(c1.srgb.G, c2.rgb.G, 0.0000001) ||
	    !ftest(c1.srgb.B, c2.rgb.B, 0.0000001)) {
		printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 1;
		goto colour_spaces_done;
	}
	if (libcolour_convert(&c2, &c1) ||
	    !ftest(c1.srgb.R, c2.rgb.R, 0.0000001) ||
	    !ftest(c1.srgb.G, c2.rgb.G, 0.0000001) ||
	    !ftest(c1.srgb.B, c2.rgb.B, 0.0000001)) {
		printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 1;
		goto colour_spaces_done;
	}

	c1.model = c2.model = LIBCOLOUR_CIELAB;
	c1.cielab.L =  30, c2.cielab.L =  80;
	c1.cielab.a =  40, c2.cielab.a = -10;
	c1.cielab.b = -50, c2.cielab.b = -40;
	if (libcolour_delta_e(&c1, &c2, &t1) || !ftest(t1, 71.4142842854285, 0.0000001))
		printf("libcolour_delta_e failed\n"), rc = 1;

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_ADOBE_RGB, 0.3, 0.2, 0.9,
		       0.379497, 0.282430, 0.914245, 0.195429, 0.098925, 0.790020);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_APPLE_RGB, 0.3, 0.2, 0.9,
		       0.364212, 0.282789, 0.866008, 0.221570, 0.134028, 0.773782);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_BEST_RGB, 0.3, 0.2, 0.9,
		       0.345007, 0.246779, 0.736029, 0.151406, 0.064655, 0.647212);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_BETA_RGB, 0.3, 0.2, 0.9,
		       0.342837, 0.253386, 0.714198, 0.146437, 0.066824, 0.623382);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_BRUCE_RGB, 0.3, 0.2, 0.9,
		       0.368857, 0.276910, 0.915272, 0.191184, 0.096699, 0.791486);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_CIE_RGB, 0.3, 0.2, 0.9,
		       0.389293, 0.225188, 0.892857, 0.202678, 0.044608, 0.785312);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_COLORMATCH_RGB, 0.3, 0.2, 0.9,
		       0.337557, 0.274378, 0.651989, 0.186858, 0.123210, 0.581381);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_DON_RGB_4, 0.3, 0.2, 0.9,
		       0.344990, 0.251411, 0.727872, 0.150504, 0.066348, 0.638058);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB, 0.3, 0.2, 0.9,
		       0.353021, 0.286400, 0.695202, 0.196733, 0.134157, 0.630279);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB_V2, 0.3, 0.2, 0.9,
		       0.353021, 0.286400, 0.695202, 0.149594, 0.097238, 0.579927);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_EKTA_SPACE_PS5, 0.3, 0.2, 0.9,
		       0.320377, 0.229160, 0.713291, 0.127134, 0.043253, 0.622392);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_NTSC_RGB, 0.3, 0.2, 0.9,
		       0.397081, 0.310031, 1.017821, 0.235650, 0.156675, 0.906707);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_PAL_SECAM_RGB, 0.3, 0.2, 0.9,
		       0.357972, 0.272130, 0.877151, 0.208580, 0.120322, 0.769056);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_PROPHOTO_RGB, 0.3, 0.2, 0.9,
		       0.294559, 0.228864, 0.742689, 0.124735, 0.072340, 0.682655);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_SMPTE_C_RGB, 0.3, 0.2, 0.9,
		       0.363595, 0.281822, 0.890350, 0.216773, 0.131311, 0.783348);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_WIDE_GAMUT_RGB, 0.3, 0.2, 0.9,
		       0.367485, 0.237631, 0.706442, 0.170318, 0.052665, 0.614915);

	rc &= test_rgb(LIBCOLOUR_RGB_COLOUR_SPACE_LIGHTROOM_RGB, 0.3, 0.2, 0.9,
		       0.294559, 0.228864, 0.742689, 0.294559, 0.228864, 0.742689);

	/*
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_DCI_P3_D65
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_DCI_P3_THEATER
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_601_625_LINE
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_601_525_LINE
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_709
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2020
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_PQ
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_PQ
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_PQ
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_HLG
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_HLG
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_HLG
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_SGI_RGB
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_SMPTE_240M_RGB
  
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MEASUREMENTS
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MATRIX
	  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_INV_MATRIX
	*/

	/* TODO test transfer functions more rigorously */

	c1.model = c2.model = LIBCOLOUR_RGB;
	c3.model = c4.model = LIBCOLOUR_SRGB;
	c1.rgb.with_transfer = c3.srgb.with_transfer = 0;
	c2.rgb.with_transfer = c4.srgb.with_transfer = 1;
	if (libcolour_get_rgb_colour_space(&c1.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB))
		goto fail;
	if (libcolour_get_rgb_colour_space(&c2.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB))
		goto fail;
	for (t1 = 0; t1 <= 1; t1 += 0.001) {
		c1.rgb.R = c3.srgb.R = t1 - 1;
		c1.rgb.G = c3.srgb.G = t1;
		c1.rgb.B = c3.srgb.B = t1 + 1;
		if (libcolour_convert(&c1, &c2))
			goto fail;
		if (libcolour_convert(&c3, &c4))
			goto fail;
		if (!ftest(c2.rgb.R, c4.srgb.R, 0.00000000001) ||
		    !ftest(c2.rgb.G, c4.srgb.G, 0.00000000001) ||
		    !ftest(c2.rgb.B, c4.srgb.B, 0.00000000001) ||
		    !ftest(libcolour_srgb_encode(c1.rgb.R), c2.srgb.R, 0.00000000001) ||
		    !ftest(libcolour_srgb_encode(c1.rgb.G), c2.srgb.G, 0.00000000001) ||
		    !ftest(libcolour_srgb_encode(c1.rgb.B), c2.srgb.B, 0.00000000001)) {
			printf("libcolour_srgb_encode failed\n"), rc = 1;
			goto colour_spaces_done;
		}
		if (libcolour_convert(&c2, &c1))
			goto fail;
		if (libcolour_convert(&c4, &c3))
			goto fail;
		if (!ftest(c1.rgb.R, c3.srgb.R, 0.00000000001) ||
		    !ftest(c1.rgb.G, c3.srgb.G, 0.00000000001) ||
		    !ftest(c1.rgb.B, c3.srgb.B, 0.00000000001) ||
		    !ftest(libcolour_srgb_decode(c2.rgb.R), c1.srgb.R, 0.00000000001) ||
		    !ftest(libcolour_srgb_decode(c2.rgb.G), c1.srgb.G, 0.00000000001) ||
		    !ftest(libcolour_srgb_decode(c2.rgb.B), c1.srgb.B, 0.00000000001)) {
			printf("libcolour_srgb_decode failed\n"), rc = 1;
			goto colour_spaces_done;
		}
	}

	c1.rgb.encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
	c1.rgb.GAMMA = 2.2;
	c1.rgb.with_transfer = 0;
	c1.rgb.R = 0.25;
	c1.rgb.G = 0.5;
	c1.rgb.B = 0.75;
	c4 = c3 = c2 = c1;
	c2.rgb.with_transfer = c3.srgb.with_transfer = 1;
	c3.rgb.GAMMA = c4.rgb.GAMMA = 1.8;
	if (libcolour_convert(&c1, &c2))
		goto fail;
	if (libcolour_convert(&c2, &c3))
		goto fail;
	if (libcolour_convert(&c3, &c4))
		goto fail;
	if (!ftest(c1.rgb.R, c4.srgb.R, 0.0000001) ||
	    !ftest(c1.rgb.G, c4.srgb.G, 0.0000001) ||
	    !ftest(c1.rgb.B, c4.srgb.B, 0.0000001)) {
		printf("libcolour_convert failed to convert between two transfer functions\n"), rc = 1;
		goto colour_spaces_done;
	}

	if (libcolour_convert(&c1, &c4))
		goto fail;
	if (!ftest(c1.rgb.R, c4.srgb.R, 0.0000001) ||
	    !ftest(c1.rgb.G, c4.srgb.G, 0.0000001) ||
	    !ftest(c1.rgb.B, c4.srgb.B, 0.0000001)) {
		printf("libcolour_convert failed to convert when two different transfer functions are not applied\n"), rc = 1;
		goto colour_spaces_done;
	}

#define TEST(TO, CH1, CH2, CH3, D1, D2, D3)\
	r = test_1convert(&c1, TO, "LIBCOLOUR_SRGB", #TO, CH1, CH2, CH3, D1, D2, D3);\
	if (r < 0)\
		goto fail;\
	if (!r)\
		rc = 1;

	c1.model = LIBCOLOUR_SRGB;
	c1.srgb.with_transfer = 0;
	c1.srgb.R = 0.3, c1.srgb.G = 0.2, c1.srgb.B = 0.7;
	TEST(LIBCOLOUR_CIEXYZ, 0.321558, 0.257355, 0.694851, 0.000001, 0.000001, 0.000001);
	TEST(LIBCOLOUR_CIEXYY, 0.252447, 0.202043, 0.257355, 0.000001, 0.000001, 0.000001);
	TEST(LIBCOLOUR_CIELAB, 57.7851, 30.3599, -44.9740, 0.0001, 0.0001, 0.0001);
	TEST(LIBCOLOUR_CIELUV, 57.7851, 5.5723, -74.1571, 0.0001, 0.0001, 0.0001);
	TEST(LIBCOLOUR_CIELCHUV, 57.7851, 74.3662, 274.2972, 0.0001, 0.0001, 0.0001);
	/*
	 * TODO convert test from sRGB to:
	 *     LChab  57.7851, 54.2622, 304.0215
	 *     YDbDr
	 *     YPbPr
	 *     YCgCo
	 *     CIE 1960 UCS
	 *     CIEUVW
	 *     YES
	 */
	c1.srgb.with_transfer = 0;
	c1.srgb.R = 32 / 255., c1.srgb.G = 65 / 255., c1.srgb.B = 32 / 255.;
	TEST(LIBCOLOUR_YIQ, 0.201, -0.035, -0.068, 0.001, 0.001, 0.001);
	TEST(LIBCOLOUR_YUV, 0.201, -0.037, -0.067, 0.001, 0.001, 0.001);
	/* XXX YCbCr(60, 118, 116) */
	/* XXX HSV(120, 0.508, 0.255) */
	/* XXX HSI(120, 0.256, 0.169) */
	/* XXX HSL(120, 0.340, 0.190) */

	if (rc)
		goto colour_spaces_done;

#undef TEST
colour_spaces_done:

	memset(&c1, 0, sizeof(c1));
	memset(&c2, 0, sizeof(c2));
	c1.model = LIBCOLOUR_RGB;
	if (libcolour_get_rgb_colour_space(&c1.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB_V2))
		goto fail;
	c3 = c1;
	c1.rgb.TO_ENCODED_RED = NULL;
	c1.rgb.TO_ENCODED_GREEN = NULL;
	c1.rgb.TO_ENCODED_BLUE = NULL;
	c1.rgb.TO_DECODED_RED = NULL;
	c1.rgb.TO_DECODED_GREEN = NULL;
	c1.rgb.TO_DECODED_BLUE = NULL;
	if (libcolour_marshal(&c1, NULL) > sizeof(buf)) {
		printf("libcolour_marshal failed\n"), rc = 1;
		goto marshal_done;
	}
	n = libcolour_marshal(&c1, buf);
	if (n > sizeof(buf)) {
		printf("libcolour_marshal failed\n"), rc = 1;
		goto marshal_done;
	}
	if (libcolour_unmarshal(NULL, buf) != n) {
		printf("libcolour_unmarshal failed\n"), rc = 1;
		goto marshal_done;
	}
	if (libcolour_unmarshal(&c2, buf) != n) {
		printf("libcolour_unmarshal failed\n"), rc = 1;
		goto marshal_done;
	}
	if (memcmp(&c2, &c3, sizeof(c2))) {
		printf("libcolour_(un)marshal failed\n"), rc = 1;
		goto marshal_done;
	}
marshal_done:

	c1.srgb.model = LIBCOLOUR_SRGB;
	c1.srgb.R = 0.8;
	c1.srgb.G = 0.7;
	c1.srgb.B = 0.6;
	c1.srgb.with_transfer = 1;
	c2.srgb.model = LIBCOLOUR_SRGB;
	c2.srgb.R = 0.1;
	c2.srgb.G = 0.2;
	c2.srgb.B = 0.3;
	c2.srgb.with_transfer = 1;
	c3.model = LIBCOLOUR_CIEXYZ;
	for (i = 0; i < 15; i++) {
		if (test_en_masse(&c1, &c2, &c3, i)) {
			rc = 1;
			goto en_masse_done;
		}
	}

	if (test_partitioned_en_masse()) {
		rc = 1;
		goto en_masse_done;
	}
en_masse_done:

	return rc;
fail:
	perror(*argv);
	(void) argc;
	return 2;
}
