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

#include <stdio.h>


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
  case LIBCOLOUR_CIELUV:
  case LIBCOLOUR_CIELCHUV:
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
    c1.srgb.R = ch1 * 100, c1.srgb.G = ch2 * 100, c1.srgb.B = ch3 * 100;
    break;
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
    case LIBCOLOUR_CIELUV:
    case LIBCOLOUR_CIELCHUV:
      c1.cieluv.white.model = LIBCOLOUR_CIEXYZ;
      if (run == 0) {
	c1.cieluv.white.X = 1.0294;
	c1.cieluv.white.Y = 1;
	c1.cieluv.white.Z = 0.9118;
      } else if (run == 1) {
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

  r = test_convert_nm_all();
  if (r < 0)
    goto fail;
  if (!r)
    rc = 1;

  /* TODO test libcolour_srgb_encode */
  /* TODO test libcolour_srgb_decode */
  /* TODO test libcolour_srgb -> libcolour_rgb[srgb] */
  /* TODO test libcolour_rgb[srgb] -> libcolour_srgb */
  /* TODO test RGB colour spaces */
  /* TODO test libcolour_delta_e */
  /* TODO test libcolour_marshal */
  /* TODO test libcolour_unmarshal */
  /* TODO test libcolour_convert with single conversions */

  return rc;
 fail:
  perror(*argv);
  return 2;
  (void) argc;
}
