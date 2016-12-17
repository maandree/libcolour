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

#include <alloca.h>
#include <errno.h>
#include <math.h>
#include <string.h>


#define MARSHAL_VERSION  0



double libcolour_srgb_encode(double t)
{
  double sign = 1;
  if (t < 0) {
    t = -t;
    sign = -1;
  }
  t = t <= 0.0031306684425217108 ? 12.92 * t : 1.055 * pow(t, 1 / 2.4) - 0.055;
  return t * sign;
}

double libcolour_srgb_decode(double t)
{
  double sign = 1;
  if (t < 0) {
    t = -t;
    sign = -1;
  }
  t = t <= 0.040448236277380506 ? t / 12.92 : pow((t + 0.055) / 1.055, 2.4);
  return t * sign;
}


int libcolour_delta_e(const libcolour_colour_t* a, const libcolour_colour_t* b, double* e)
{
  libcolour_cielab_t u, v;
  u.model = v.model = LIBCOLOUR_CIELAB;
  if (libcolour_convert(a, (libcolour_colour_t*)&u))
    return -1;
  if (libcolour_convert(b, (libcolour_colour_t*)&v))
    return -1;
  u.L -= v.L, u.L *= u.L;
  u.a -= v.a, u.a *= u.a;
  u.b -= v.b, u.b *= u.b;
  *e = sqrt(u.L + u.a + u.b);
  return 0;
}


#define eliminate(M, n, m)  eliminate_(n, m, &(M))
static int eliminate_(size_t n, size_t m, double (*Mp)[n][m])
{
#define M (*Mp)
  size_t r1, r2, c;
  double d;
  double* R1;
  double* R2;
  for (r1 = 0; r1 < n; r1++) {
    R1 = M[r1];
    if (R1[r1] == 0) {
      for (r2 = r1 + 1; r2 < n; r2++)
	if (M[r2][r1] != 0)
	  break;
      if (r2 == n) {
	errno = EDOM;
	return -1;
      }
      R2 = M[r2];
      for (c = 0; c < m; c++)
	d = R1[c], R1[c] = R2[c], R2[c] = d;
    }
    d = R1[r1];
    for (c = 0; c < m; c++)
      R1[c] /= d;
    for (r2 = r1 + 1; r2 < n; r2++) {
      R2 = M[r2];
      d = R2[r1];
      for (c = 0; c < m; c++)
	R2[c] -= R1[c] * d;
    }
  }
  for (r1 = n; --r1;) {
    R1 = M[r1];
    for (r2 = r1; r2--;) {
      R2 = M[r2];
      d = R2[r1];
      for (c = 0; c < m; c++)
	R2[c] -= R1[c] * d;
    }
  }
  return 0;
#undef M
}


int libcolour_proper(libcolour_colour_t* colour)
{
  libcolour_colour_t r, g, b, w;
  double m[3][4];
  switch (colour->model) {
  case LIBCOLOUR_CIELUV:
    colour->cieluv.white.model = LIBCOLOUR_CIEXYZ;
    break;
  case LIBCOLOUR_CIELCHUV:
    colour->cielchuv.white.model = LIBCOLOUR_CIEXYZ;
    if (colour->cielchuv.one_revolution == 0.)
      colour->cielchuv.one_revolution = 360.;
    break;
  case LIBCOLOUR_RGB:
    colour->rgb.red.model   = LIBCOLOUR_CIEXYY;
    colour->rgb.green.model = LIBCOLOUR_CIEXYY;
    colour->rgb.blue.model  = LIBCOLOUR_CIEXYY;
    colour->rgb.white.model = LIBCOLOUR_CIEXYY;
    colour->rgb.red.Y   = 1;
    colour->rgb.green.Y = 1;
    colour->rgb.blue.Y  = 1;
    r.model = g.model = b.model = w.model = LIBCOLOUR_CIEXYZ;
    libcolour_convert((const libcolour_colour_t*)&colour->rgb.red, &r);
    libcolour_convert((const libcolour_colour_t*)&colour->rgb.green, &g);
    libcolour_convert((const libcolour_colour_t*)&colour->rgb.blue, &b);
    libcolour_convert((const libcolour_colour_t*)&colour->rgb.white, &w);
    m[0][0] = r.ciexyz.X, m[0][1] = g.ciexyz.X, m[0][2] = b.ciexyz.X, m[0][3] = w.ciexyz.X;
    m[1][0] = r.ciexyz.Y, m[1][1] = g.ciexyz.Y, m[1][2] = b.ciexyz.Y, m[1][3] = w.ciexyz.Y;
    m[2][0] = r.ciexyz.Z, m[2][1] = g.ciexyz.Z, m[2][2] = b.ciexyz.Z, m[2][3] = w.ciexyz.Z;
    if (eliminate(m, 3, 4))
      return -1;
    colour->rgb.red.Y   = m[0][3];
    colour->rgb.green.Y = m[1][3];
    colour->rgb.blue.Y  = m[2][3];
    break;
  default:
    break;
  }
  return 0;
}


static int get_primaries(libcolour_rgb_t* cs)
{
  libcolour_colour_t r, g, b, w;
  double Sr, Sg, Sb;
  double M[3][6];

  r.model = g.model = b.model = w.model = LIBCOLOUR_CIEXYZ;
  r.ciexyz.Y = 1;
  g.ciexyz.Y = 1;
  b.ciexyz.Y = 1;

  Sr = cs->M[1][0] * cs->white_r;
  Sg = cs->M[1][1] * cs->white_g;
  Sb = cs->M[1][2] * cs->white_b;

  r.ciexyz.X = cs->M[0][0] / Sr, r.ciexyz.Z = cs->M[2][0] / Sr;
  g.ciexyz.X = cs->M[0][1] / Sg, g.ciexyz.Z = cs->M[2][1] / Sg;
  b.ciexyz.X = cs->M[0][2] / Sb, b.ciexyz.Z = cs->M[2][2] / Sb;

  M[0][0] = r.ciexyz.X, M[0][1] = g.ciexyz.X, M[0][2] = b.ciexyz.X, M[0][3] = 1, M[0][4] = 0, M[0][5] = 0;
  M[1][0] = r.ciexyz.Y, M[1][1] = g.ciexyz.Y, M[1][2] = b.ciexyz.Y, M[1][3] = 0, M[1][4] = 1, M[1][5] = 0;
  M[2][0] = r.ciexyz.Z, M[2][1] = g.ciexyz.Z, M[2][2] = b.ciexyz.Z, M[2][3] = 0, M[2][4] = 0, M[2][5] = 1;

  if (eliminate(M, 3, 6))
    return -1;

  memcpy(M[0], M[0] + 3, 3 * sizeof(double)), M[0][3] = Sr;
  memcpy(M[1], M[1] + 3, 3 * sizeof(double)), M[1][3] = Sg;
  memcpy(M[2], M[2] + 3, 3 * sizeof(double)), M[2][3] = Sb;

  if (eliminate(M, 3, 4))
    return -1;

  w.ciexyz.X = M[0][3];
  w.ciexyz.Y = M[1][3];
  w.ciexyz.Z = M[2][3];

  cs->red.model   = LIBCOLOUR_CIEXYY;
  cs->green.model = LIBCOLOUR_CIEXYY;
  cs->blue.model  = LIBCOLOUR_CIEXYY;
  cs->white.model = LIBCOLOUR_CIEXYY;

  libcolour_convert(&r, (libcolour_colour_t*)&cs->red);
  libcolour_convert(&g, (libcolour_colour_t*)&cs->green);
  libcolour_convert(&b, (libcolour_colour_t*)&cs->blue);
  libcolour_convert(&w, (libcolour_colour_t*)&cs->white);

  return 0;
}


static int get_matrices(libcolour_rgb_t* cs)
{
  libcolour_colour_t r, g, b, w;
  double M[3][6];
  double Sr, Sg, Sb;

  r.model = g.model = b.model = LIBCOLOUR_CIEXYZ;
  w.model = LIBCOLOUR_CIEXYY;
  libcolour_convert((const libcolour_colour_t*)&cs->red,   &w), w.ciexyy.Y = 1, libcolour_convert(&w, &r);
  libcolour_convert((const libcolour_colour_t*)&cs->green, &w), w.ciexyy.Y = 1, libcolour_convert(&w, &g);
  libcolour_convert((const libcolour_colour_t*)&cs->blue,  &w), w.ciexyy.Y = 1, libcolour_convert(&w, &b);
  w.model = LIBCOLOUR_CIEXYZ;
  libcolour_convert((const libcolour_colour_t*)&cs->white, &w);

  M[0][0] = r.ciexyz.X, M[0][1] = g.ciexyz.X, M[0][2] = b.ciexyz.X, M[0][3] = 1, M[0][4] = 0, M[0][5] = 0;
  M[1][0] = r.ciexyz.Y, M[1][1] = g.ciexyz.Y, M[1][2] = b.ciexyz.Y, M[1][3] = 0, M[1][4] = 1, M[1][5] = 0;
  M[2][0] = r.ciexyz.Z, M[2][1] = g.ciexyz.Z, M[2][2] = b.ciexyz.Z, M[2][3] = 0, M[2][4] = 0, M[2][5] = 1;

  if (eliminate(M, 3, 6))
    return -1;

  Sr = M[0][3] * w.ciexyz.X + M[0][4] * w.ciexyz.Y + M[0][5] * w.ciexyz.Z;
  Sg = M[1][3] * w.ciexyz.X + M[1][4] * w.ciexyz.Y + M[1][5] * w.ciexyz.Z;
  Sb = M[2][3] * w.ciexyz.X + M[2][4] * w.ciexyz.Y + M[2][5] * w.ciexyz.Z;

  Sr /= cs->white_r;
  Sg /= cs->white_g;
  Sb /= cs->white_b;

  cs->M[0][0] = Sr * r.ciexyz.X, cs->M[0][1] = Sg * g.ciexyz.X, cs->M[0][2] = Sb * b.ciexyz.X;
  cs->M[1][0] = Sr * r.ciexyz.Y, cs->M[1][1] = Sg * g.ciexyz.Y, cs->M[1][2] = Sb * b.ciexyz.Y;
  cs->M[2][0] = Sr * r.ciexyz.Z, cs->M[2][1] = Sg * g.ciexyz.Z, cs->M[2][2] = Sb * b.ciexyz.Z;

  memcpy(M[0], cs->M[0], 3 * sizeof(double)), M[0][3] = 1, M[0][4] = 0, M[0][5] = 0;
  memcpy(M[1], cs->M[1], 3 * sizeof(double)), M[1][3] = 0, M[1][4] = 1, M[1][5] = 0;
  memcpy(M[2], cs->M[2], 3 * sizeof(double)), M[2][3] = 0, M[2][4] = 0, M[2][5] = 1;

  if (eliminate(M, 3, 6))
    return -1;

  memcpy(cs->Minv[0], M[0] + 3, 3 * sizeof(double));
  memcpy(cs->Minv[1], M[1] + 3, 3 * sizeof(double));
  memcpy(cs->Minv[2], M[2] + 3, 3 * sizeof(double));

  return 0;
}


#define invert(Minv, M, n)  invert_(n, &(Minv), &(M))
static int invert_(size_t n, double (*Minvp)[n][n], double (*Mp)[n][n])
{
#define Minv (*Minvp)
#define M (*Mp)
#define J (*Jp)
  double J[n][2 * n] = alloca(sizeof(double[n][2 * n]));
  size_t i;
  for (i = 0; i < n; i++) {
    memcpy(J[i], M[i], n * sizeof(double));
    memset(J[i] + n, 0, n * sizeof(double));
    J[i][n + i] = 1;
  }
  if (eliminate(J, n, 2 * n))
    return -1;
  for (i = 0; i < n; i++)
    memcpy(Minv[i], J[i] + n, n * sizeof(double));
  return 0;
#undef J
#undef M
#undef Minv
}


static double transfer_function_l_star(double t)
{
  double sign = 1;
  if (t < 0) {
    t = -t;
    sign = -1;
  }
  t = t > 216. / 24389. ? 1.16 * cbrt(t) - 0.16 : t * 24389. / 2700.;
  return t * sign;
}
 
static double invtransfer_function_l_star(double t)
{
  double sign = 1;
  if (t < 0) {
    t = -t;
    sign = -1;
  }
  t = t > 0.08 ?  (((1000000. * t + 480000.) * t + 76800.) * t + 4096.) / 1560896.0 : t * 2700. / 24389.;
  return t * sign;
}

static double transfer_function_oetf_hlg(double t)
{
  double sign = 1;
  if (t < 0) {
    t = -t;
    sign = -1;
  }
  t = 12 * t <= 1 ? sqrt(3 * t) : 0.17883277 * log(t - 0.02372241) + 1.004293468902569985701234145381;
  return t * sign;
}

static double invtransfer_function_oetf_hlg(double t)
{
  double sign = 1;
  if (t < 0) {
    t = -t;
    sign = -1;
  }
  t = t <= 0.5 ? t * t / 3 : exp(t - 1.004293468902569985701234145381) / 0.17883277 + 0.02372241;
  return t * sign;
}


static void get_transfer_function(libcolour_colour_t* cs)
{
  if (cs->model == LIBCOLOUR_RGB) {
    switch (cs->rgb.colour_space) {
    case LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB_V2:
      cs->rgb.to_encoded_red = cs->rgb.to_encoded_green = cs->rgb.to_encoded_blue = transfer_function_l_star;
      cs->rgb.to_decoded_red = cs->rgb.to_decoded_green = cs->rgb.to_decoded_blue = invtransfer_function_l_star;
      break;
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_PQ:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_PQ:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_PQ:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_HLG:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_HLG:
      /* TODO http://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2100-0-201607-I!!PDF-E.pdf */
      break;
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_HLG:
      cs->rgb.to_encoded_red = cs->rgb.to_encoded_green = cs->rgb.to_encoded_blue = transfer_function_oetf_hlg;
      cs->rgb.to_decoded_red = cs->rgb.to_decoded_green = cs->rgb.to_decoded_blue = invtransfer_function_oetf_hlg;
      break;
    default:
      break;
    }
  }
}


int libcolour_get_rgb_colour_space(libcolour_rgb_t* cs, libcolour_rgb_colour_space_t space)
{
#define XYY(XVALUE, YVALUE)  (libcolour_ciexyy_t){ .model = LIBCOLOUR_CIEXYY, .x = XVALUE, .y = YVALUE, .Y = 1}

  switch (space) {
  case LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MEASUREMENTS:
    if (get_matrices(cs))
      return -1;
    if (cs->encoding_type == LIBCOLOUR_ENCODING_TYPE_REGULAR)
      cs->transitioninv = cs->transition * cs->slope;
    return 0;

  case LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MATRIX:
    if (invert(cs->Minv, cs->M, 3) || get_primaries(cs))
      return -1;
    if (cs->encoding_type == LIBCOLOUR_ENCODING_TYPE_REGULAR)
      cs->transitioninv = cs->transition * cs->slope;
    return 0;

  case LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_INV_MATRIX:
    if (invert(cs->M, cs->Minv, 3) || get_primaries(cs))
      return -1;
    if (cs->encoding_type == LIBCOLOUR_ENCODING_TYPE_REGULAR)
      cs->transitioninv = cs->transition * cs->slope;
    return 0;

  case LIBCOLOUR_RGB_COLOUR_SPACE_SRGB:
    cs->red   = XYY(0.6400, 0.3300);
    cs->green = XYY(0.3000, 0.6000);
    cs->blue  = XYY(0.1500, 0.0600);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->offset = 0.055;
    cs->gamma = 2.4;
    cs->slope = 12.92;
    cs->transition = 0.0031306684425217108;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_ADOBE_RGB:
    cs->red   = XYY(0.6400, 0.3300);
    cs->green = XYY(0.2100, 0.7100);
    cs->blue  = XYY(0.1500, 0.0600);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.2;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_APPLE_RGB:
    cs->red   = XYY(0.6250, 0.3400);
    cs->green = XYY(0.2800, 0.5950);
    cs->blue  = XYY(0.1550, 0.0700);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 1.8;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_BEST_RGB:
    cs->red   = XYY(0.7347, 0.2653);
    cs->green = XYY(0.2150, 0.7750);
    cs->blue  = XYY(0.1300, 0.0350);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.2;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_BETA_RGB:
    cs->red   = XYY(0.6888, 0.3112);
    cs->green = XYY(0.1986, 0.7551);
    cs->blue  = XYY(0.1265, 0.0352);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.2;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_BRUCE_RGB:
    cs->red   = XYY(0.6400, 0.3300);
    cs->green = XYY(0.2800, 0.6500);
    cs->blue  = XYY(0.1500, 0.0600);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.2;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_CIE_RGB:
    cs->red   = XYY(0.7350, 0.2650);
    cs->green = XYY(0.2740, 0.7170);
    cs->blue  = XYY(0.1670, 0.0090);
    cs->white = LIBCOLOUR_ILLUMINANT_E;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.2;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_COLORMATCH_RGB:
    cs->red   = XYY(0.6300, 0.3400);
    cs->green = XYY(0.2950, 0.6050);
    cs->blue  = XYY(0.1500, 0.0750);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 1.8;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_DCI_P3_D65:
    cs->red   = XYY(0.6380, 0.3200);
    cs->green = XYY(0.2650, 0.6900);
    cs->blue  = XYY(0.1500, 0.0600);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.6;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_DCI_P3_THEATER:
    cs->red   = XYY(0.6380, 0.3200);
    cs->green = XYY(0.2650, 0.6900);
    cs->blue  = XYY(0.1500, 0.0600);
    cs->white = XYY(0.314, 0.351);
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.6;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_DON_RGB_4:
    cs->red   = XYY(0.6960, 0.3000);
    cs->green = XYY(0.2150, 0.7650);
    cs->blue  = XYY(0.1300, 0.0350);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.2;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB:
    cs->red   = XYY(0.6700, 0.3300);
    cs->green = XYY(0.2100, 0.7100);
    cs->blue  = XYY(0.1400, 0.0800);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 1.8;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB_V2:
    cs->red   = XYY(0.6700, 0.3300);
    cs->green = XYY(0.2100, 0.7100);
    cs->blue  = XYY(0.1400, 0.0800);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_CUSTOM;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_EKTA_SPACE_PS5:
    cs->red   = XYY(0.6950, 0.3050);
    cs->green = XYY(0.2600, 0.7000);
    cs->blue  = XYY(0.1100, 0.0050);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.2;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_601_625_LINE:
    cs->red   = XYY(0.6400, 0.3300);
    cs->green = XYY(0.2900, 0.6000);
    cs->blue  = XYY(0.1500, 0.0060);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->gamma = 1 / 0.45;
    cs->offset = 0.09929682680944;
    cs->slope = 4.5;
    cs->transition = 0.018053968510807;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_601_525_LINE:
    cs->red   = XYY(0.6300, 0.3400);
    cs->green = XYY(0.3100, 0.5950);
    cs->blue  = XYY(0.1550, 0.0700);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->gamma = 1 / 0.45;
    cs->offset = 0.09929682680944;
    cs->slope = 4.5;
    cs->transition = 0.018053968510807;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_709:
    cs->red   = XYY(0.6300, 0.3300);
    cs->green = XYY(0.3000, 0.6000);
    cs->blue  = XYY(0.1500, 0.0600);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->gamma = 1 / 0.45;
    cs->offset = 0.09929682680944;
    cs->slope = 4.5;
    cs->transition = 0.018053968510807;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2020:
    cs->red   = XYY(0.7080, 0.2920);
    cs->green = XYY(0.1700, 0.7970);
    cs->blue  = XYY(0.1310, 0.0460);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->gamma = 1 / 0.45;
    cs->offset = 0.09929682680944;
    cs->slope = 4.5;
    cs->transition = 0.018053968510807;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_PQ:
  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_PQ:
  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_PQ:
  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_HLG:
  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_HLG:
  case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_HLG:
    cs->red   = XYY(0.7080, 0.2920);
    cs->green = XYY(0.1700, 0.7970);
    cs->blue  = XYY(0.1310, 0.0460);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_CUSTOM;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_LIGHTROOM_RGB:
    cs->red   = XYY(0.7347, 0.2653);
    cs->green = XYY(0.1596, 0.8404);
    cs->blue  = XYY(0.0366, 0.0001);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_LINEAR;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_NTSC_RGB:
    cs->red   = XYY(0.6700, 0.3300);
    cs->green = XYY(0.2100, 0.7100);
    cs->blue  = XYY(0.1400, 0.0800);
    cs->white = LIBCOLOUR_ILLUMINANT_C;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->gamma = 1 / 0.45;
    cs->offset = 0.09929682680944;
    cs->slope = 4.5;
    cs->transition = 0.018053968510807;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_PAL_SECAM_RGB:
    cs->red   = XYY(0.6400, 0.3300);
    cs->green = XYY(0.2900, 0.6000);
    cs->blue  = XYY(0.1500, 0.0600);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->gamma = 1 / 0.45;
    cs->offset = 0.09929682680944;
    cs->slope = 4.5;
    cs->transition = 0.018053968510807;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_PROPHOTO_RGB:
    cs->red   = XYY(0.7347, 0.2653);
    cs->green = XYY(0.1596, 0.8404);
    cs->blue  = XYY(0.0366, 0.0001);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 1.8;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_SGI_RGB:
    cs->red   = XYY(0.6250, 0.3400);
    cs->green = XYY(0.2800, 0.5950);
    cs->blue  = XYY(0.1550, 0.0700);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 1.47;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_SMPTE_240M_RGB:
    cs->red   = XYY(0.6300, 0.3400);
    cs->green = XYY(0.3100, 0.5950);
    cs->blue  = XYY(0.1550, 0.0700);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->gamma = 1 / 0.45;
    cs->offset = 0.1115721957735072;
    cs->slope = 4.0;
    cs->transition = 0.022821585552393633;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_SMPTE_C_RGB:
    cs->red   = XYY(0.6300, 0.3400);
    cs->green = XYY(0.3100, 0.5950);
    cs->blue  = XYY(0.1550, 0.0700);
    cs->white = LIBCOLOUR_ILLUMINANT_D65;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_REGULAR;
    cs->gamma = 1 / 0.45;
    cs->offset = 0.09929682680944;
    cs->slope = 4.5;
    cs->transition = 0.018053968510807;
    break;

  case LIBCOLOUR_RGB_COLOUR_SPACE_WIDE_GAMUT_RGB:
    cs->red   = XYY(0.7350, 0.2650);
    cs->green = XYY(0.1150, 0.8260);
    cs->blue  = XYY(0.1570, 0.0180);
    cs->white = LIBCOLOUR_ILLUMINANT_D50;
    cs->encoding_type = LIBCOLOUR_ENCODING_TYPE_SIMPLE;
    cs->gamma = 2.19921875;
    break;

  default:
    errno = EINVAL;
    return -1;
  }
  if (cs->encoding_type == LIBCOLOUR_ENCODING_TYPE_REGULAR)
    cs->transitioninv = cs->transition * cs->slope;
  cs->colour_space = space;
  cs->white_r = cs->white_g = cs->white_b = 1;
  if (get_matrices(cs) || libcolour_proper((libcolour_colour_t*)cs))
    return -1;
  get_transfer_function((libcolour_colour_t*)cs);
  return 0;

#undef XYY
}


size_t libcolour_marshal(const libcolour_colour_t* colour, void* buf)
{
  if (buf)
    *(int*)buf = MARSHAL_VERSION;
  switch (colour->model) {
#define X(C, T)\
  case C:\
    if (buf)\
      memcpy((char*)buf + sizeof(int), colour, sizeof(T));\
    return sizeof(int) + sizeof(T);
  LIBCOLOUR_LIST_MODELS
#undef X
  default:
    errno = EINVAL;
    return 0;
  }
}


size_t libcolour_unmarshal(libcolour_colour_t* colour, const void* buf)
{
  enum libcolour_model model;
  size_t r;
  int ver;
  ver = *(int*)buf;
  if (ver != MARSHAL_VERSION) {
    errno = EINVAL;
    return 0;
  }
  model = *(enum libcolour_model*)((char*)buf + sizeof(int));
  switch (model) {
#define X(C, T)\
    case C:\
      if (colour)\
	memcpy(colour, (char*)buf + sizeof(int), sizeof(T));\
      r = sizeof(int) + sizeof(T);\
      break;
  LIBCOLOUR_LIST_MODELS
#undef X
  default:
    errno = EINVAL;
    return 0;
  }
  if (colour) {
    if (colour->model == LIBCOLOUR_RGB) {
      colour->rgb.to_encoded_red = colour->rgb.to_encoded_green = colour->rgb.to_encoded_blue = NULL;
      colour->rgb.to_decoded_red = colour->rgb.to_decoded_green = colour->rgb.to_decoded_blue = NULL;
    }
    get_transfer_function(colour);
  }
  return r;
}

