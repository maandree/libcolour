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


int test_convert_(libcolour_colour_t* c1, libcolour_colour_t* c2, libcolour_colour_t* c3)
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
  if (ch1 > 0.0000001 ||
      ch2 > 0.0000001 ||
      ch3 > 0.0000001)
    return 0;
  return 1;
}
  

int test_convert(libcolour_colour_t* c1, libcolour_model_t model)
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
    return test_convert_(c1, &c2, &c3);
  case LIBCOLOUR_SRGB:
    c2.srgb.with_gamma = 0;
    if (r1 = test_convert_(c1, &c2, &c3), r1 < 0)
      return -1;
    c2.srgb.with_gamma = 1;
    if (r2 = test_convert_(c1, &c2, &c3), r2 < 0)
      return -1;
    return r1 & r2;
  case LIBCOLOUR_CIELUV:
  case LIBCOLOUR_CIELCH:
    c2.cielch.white.model = LIBCOLOUR_CIEXYZ;
    c2.cielch.white.X = 1.0294;
    c2.cielch.white.Y = 1;
    c2.cielch.white.Z = 0.9118;
    return test_convert_(c1, &c2, &c3);
  case LIBCOLOUR_CIEUVW:
    c2.cieuvw.u0 = 0.37;
    c2.cieuvw.v0 = 0.30;
    return test_convert_(c1, &c2, &c3);
  default:
    return test_convert_(c1, &c2, &c3);
  }
}


int test_convert_all(libcolour_model_t model, const char* model_name)
{
  libcolour_colour_t c1;
  int r, rc = 1;

  c1.model = model;
  c1.srgb.R = 0.2, c1.srgb.G = 0.5, c1.srgb.B = 0.9;

  switch (model) {
  case LIBCOLOUR_RGB:
    c1.rgb.with_gamma = 0;
    if (libcolour_get_rgb_colour_space(&c1.rgb, LIBCOLOUR_RGB_COLOUR_SPACE_SRGB) < 0)
      return -1;
    break;
  case LIBCOLOUR_SRGB:
    c1.srgb.with_gamma = 0;
    break;
  case LIBCOLOUR_CIELUV:
  case LIBCOLOUR_CIELCH:
    c1.cielch.white.model = LIBCOLOUR_CIEXYZ;
    c1.cielch.white.X = 1.0294;
    c1.cielch.white.Y = 1;
    c1.cielch.white.Z = 0.9118;
    break;
  case LIBCOLOUR_CIEUVW:
    c1.cieuvw.u0 = 0.37;
    c1.cieuvw.v0 = 0.30;
    break;
  default:
    break;
  }

  c1.srgb.with_gamma = 0;
#define X(ENUM, TYPE)\
  r = test_convert(&c1, ENUM);\
  if (r < 0)\
    return -1;\
  if (!r)\
    printf("%s -> %s -> %s failed\n", model_name, #ENUM, model_name), rc = 0;
  LIBCOLOUR_LIST_MODELS;
#undef X

  return rc;
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

#define X(ENUM, TYPE)\
  r = test_convert_all(ENUM, #ENUM);\
  if (r < 0)\
    goto fail;\
  if (!r)\
    rc = 1;
  LIBCOLOUR_LIST_MODELS;
#undef X

  return rc;
 fail:
  perror(*argv);
  return 2;
  (void) argc;
}
