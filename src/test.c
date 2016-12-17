/**
 * Copyright © 2016  Mattias Andrée <maandree@kth.se>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "libcolour.h"

#include <math.h>
#include <stdio.h>


#ifndef SKIP_CONVERT
static int test_convert(libcolour_colour_t* c1, libcolour_colour_t* c2, libcolour_colour_t* c3)
{
  double ch1, ch2, ch3;
  if (libcolour_convert(c1, c2))
    return -1;
  if (libcolour_convert(c2, c3))
    return -1;
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
  

static int test_convert_11(libcolour_colour_t* c1, libcolour_model_t model)
{
  libcolour_colour_t c2, c3;
  int r1, r2;
  c3 = *c1;
  c2.model = model;
  switch (model) {
  case LIBCOLOUR_RGB:
    c2.rgb.with_gamma = 0;
    if (libcolour_get_rgb_colour_space(&c2.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB) < 0)
      return -1;
    return test_convert(c1, &c2, &c3);
  case LIBCOLOUR_SRGB:
    c2.srgb.with_gamma = 0;
    if (r1 = test_convert(c1, &c2, &c3), r1 < 0)
      return -1;
    c2.srgb.with_gamma = 1;
    if (r2 = test_convert(c1, &c2, &c3), r2 < 0)
      return -1;
    return r1 & r2;
  case LIBCOLOUR_CIELCHUV:
    c2.cielchuv.one_revolution = 360.;
    /* fall though */
  case LIBCOLOUR_CIELUV:
    c2.cieluv.white.model = LIBCOLOUR_CIEXYZ;
    c2.cieluv.white.X = 1.0294;
    c2.cieluv.white.Y = 1;
    c2.cieluv.white.Z = 0.9118;
    return test_convert(c1, &c2, &c3);
  case LIBCOLOUR_CIEUVW:
    c2.cieuvw.u0 = 0.37;
    c2.cieuvw.v0 = 0.30;
    return test_convert(c1, &c2, &c3);
  default:
    return test_convert(c1, &c2, &c3);
  }
}


static int test_convert_1n(libcolour_model_t model, const char* model_name, double ch1, double ch2, double ch3)
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
    /* fall though */
  default:
    c1.srgb.R = ch1, c1.srgb.G = ch2, c1.srgb.B = ch3;
    break;
  }

  for (run = 0;; run++) {
    switch (model) {
    case LIBCOLOUR_RGB:
      if (run == 2)
	return rc;
      c1.rgb.with_gamma = run;
      if (libcolour_get_rgb_colour_space(&c1.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB) < 0)
	return -1;
      break;
    case LIBCOLOUR_SRGB:
      if (run == 2)
	return rc;
      c1.srgb.with_gamma = run;
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

#define X(ENUM, TYPE)\
    r = test_convert_11(&c1, ENUM);\
    if (r < 0)\
      return -1;\
    if (!r)\
      printf("%s -> %s -> %s failed at run %i with (%lf, %lf, %lf)\n",\
	     model_name, #ENUM, model_name, run, ch1, ch2, ch3), rc = 0;
    LIBCOLOUR_LIST_MODELS;
#undef X
  }
}


static int test_convert_nm(double ch1, double ch2, double ch3)
{
  int r, rc = 1;

#define X(ENUM, TYPE)\
  r = test_convert_1n(ENUM, #ENUM, ch1, ch2, ch3);\
  if (r < 0)\
    return -1;\
  if (!r)\
    rc = 0;
  LIBCOLOUR_LIST_MODELS;
#undef X

  return rc;
}


static int test_convert_nm_all(void)
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

	r = test_convert_nm(ch1, ch2, ch3);
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


static inline int ftest(double a, double b, double l)
{
  return a - l <= b && b <= a + l;
}


#define test_rgb(COLOUR_SPACE, ...)  test_rgb_(COLOUR_SPACE, #COLOUR_SPACE, __VA_ARGS__)
static int test_rgb_(enum libcolour_rgb_colour_space colour_space, const char* name,
		     double r, double g, double b, double x, double y, double z,
		     double xx, double yy, double zz)
{
  libcolour_colour_t c1, c2;

  c1.model = LIBCOLOUR_RGB;
  c2.model = LIBCOLOUR_CIEXYZ;
  c1.rgb.with_gamma = 0;
  c1.rgb.R = r, c1.rgb.G = g, c1.rgb.B = b;
  if (libcolour_get_rgb_colour_space(&c1.rgb, colour_space) ||
      libcolour_convert(&c1, &c2)) {
    printf("%s failed without gamma\n", name);
    return 0;
  } else if (!ftest(c2.ciexyz.X, x, 0.0001) ||
	     !ftest(c2.ciexyz.Y, y, 0.0001) ||
	     !ftest(c2.ciexyz.Z, z, 0.0001)) {
    printf("%s failed without gamma\n", name);
    printf("%.6lf, %.6lf, %.6lf\n", c2.ciexyz.X, c2.ciexyz.Y, c2.ciexyz.Z);
    return 0;
  }

  c1.model = LIBCOLOUR_RGB;
  c2.model = LIBCOLOUR_CIEXYZ;
  c1.rgb.with_gamma = 1;
  c1.rgb.R = r, c1.rgb.G = g, c1.rgb.B = b;
  if (libcolour_get_rgb_colour_space(&c1.rgb, colour_space) ||
      libcolour_convert(&c1, &c2)) {
    printf("%s failed with gamma\n", name);
    return 0;
  } else if (!ftest(c2.ciexyz.X, xx, 0.0001) ||
	     !ftest(c2.ciexyz.Y, yy, 0.0001) ||
	     !ftest(c2.ciexyz.Z, zz, 0.0001)) {
    printf("%s failed with gamma\n", name);
    printf("%.6lf, %.6lf, %.6lf\n", c2.ciexyz.X, c2.ciexyz.Y, c2.ciexyz.Z);
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


/**
 * Test libcolour
 * 
 * @return  0: All tests passed
 *          1: At least one test fail
 *          2: An error occurred
 */
int main(int argc, char* argv[])
{
  int r, rc = 0;
  libcolour_colour_t c1, c2;
  double t1, t2;

#ifndef SKIP_CONVERT
  r = test_convert_nm_all();
  if (r < 0)
    goto fail;
  if (!r)
    rc = 1;
#else
  (void) r;
#endif

  c1.model = LIBCOLOUR_SRGB;
  c1.srgb.R = 0.02, c1.srgb.G = 0.5, c1.srgb.B = 0.9;
  c1.srgb.with_gamma = 0;
  c2.model = LIBCOLOUR_RGB;
  c2.rgb.with_gamma = 0;
  if (libcolour_get_rgb_colour_space(&c2.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB)) {
    printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 0;
    goto colour_spaces_done;
  }
  if (c2.rgb.white_r != 1 || c2.rgb.white_g != 1 || c2.rgb.white_b != 1 ||
      c2.rgb.red.model != LIBCOLOUR_CIEXYY || c2.rgb.green.model != LIBCOLOUR_CIEXYY ||
      c2.rgb.blue.model != LIBCOLOUR_CIEXYY || c2.rgb.white.model != LIBCOLOUR_CIEXYY ||
      c2.rgb.colour_space != LIBCOLOUR_RGB_COLOUR_SPACE_SRGB || c2.rgb.gamma != 2.4 ||
      c2.rgb.encoding_type != LIBCOLOUR_ENCODING_TYPE_REGULAR || c2.rgb.slope != 12.92
      || c2.rgb.offset != 0.055 || c2.rgb.transitioninv != c2.rgb.transition * c2.rgb.slope ||
      !ftest(c2.rgb.transition, 0.00313067, 0.00000001)) {
    printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 0;
    goto colour_spaces_done;
  }
  t1 = c2.rgb.transition * c2.rgb.slope;
  t2 = (1 + c2.rgb.offset) * pow(c2.rgb.transition, 1 / c2.rgb.gamma) - c2.rgb.offset;
  if (!ftest(t1, t2, 0.0000001) || c2.rgb.white.Y != 1 ||
      c2.rgb.red.x != 0.64 || c2.rgb.red.y != 0.33 || !ftest(c2.rgb.red.Y, 0.21265, 0.00005) ||
      c2.rgb.green.x != 0.30 || c2.rgb.green.y != 0.60 || !ftest(c2.rgb.green.Y, 0.71515, 0.00005) ||
      c2.rgb.blue.x != 0.15 || c2.rgb.blue.y != 0.06 || !ftest(c2.rgb.blue.Y, 0.07215, 0.00005) ||
      !ftest(c2.rgb.white.x, 0.312727, 0.000001) || !ftest(c2.rgb.white.y, 0.329023, 0.000001) ||
      !ftest(c2.rgb.red.Y + c2.rgb.green.Y + c2.rgb.blue.Y, 1, 0.00000001)) {
    printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 0;
    goto colour_spaces_done;
  }
  if (libcolour_convert(&c1, &c2) ||
      !ftest(c1.srgb.R, c2.rgb.R, 0.0000001) ||
      !ftest(c1.srgb.G, c2.rgb.G, 0.0000001) ||
      !ftest(c1.srgb.B, c2.rgb.B, 0.0000001)) {
    printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 0;
    goto colour_spaces_done;
  }
  if (libcolour_convert(&c2, &c1) ||
      !ftest(c1.srgb.R, c2.rgb.R, 0.0000001) ||
      !ftest(c1.srgb.G, c2.rgb.G, 0.0000001) ||
      !ftest(c1.srgb.B, c2.rgb.B, 0.0000001)) {
    printf("LIBCOLOUR_RGB_COLOUR_SPACE_SRGB failed\n"), rc = 0;
    goto colour_spaces_done;
  }

  c1.model = c2.model = LIBCOLOUR_CIELAB;
  c1.cielab.L =  30, c2.cielab.L =  80;
  c1.cielab.a =  40, c2.cielab.a = -10;
  c1.cielab.b = -50, c2.cielab.b = -40;
  if (libcolour_delta_e(&c1, &c2, &t1) || !ftest(t1, 71.4142842854285, 0.0000001))
    printf("libcolour_delta_e failed\n"), rc = 0;

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
  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MEASUREMENTS
  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MATRIX
  TODO test LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_INV_MATRIX
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
  */

  /* TODO test transfer functions more rigorously */

  /* TODO test libcolour_convert with single conversions */
 colour_spaces_done:

  /* TODO test libcolour_srgb_encode */
  /* TODO test libcolour_srgb_decode */
  /* TODO test libcolour_marshal */
  /* TODO test libcolour_unmarshal */
  return rc;
 fail:
  perror(*argv);
  return 2;
  (void) argc;
}
