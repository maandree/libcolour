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

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define WASDIV0(x)  (isinf(x) || isnan(x))



static void to_rgb(const libcolour_colour_t* restrict from, libcolour_rgb_t* restrict to);
static void to_srgb(const libcolour_colour_t* restrict from, libcolour_srgb_t* restrict to);
static void to_ciexyy(const libcolour_colour_t* restrict from, libcolour_ciexyy_t* restrict to);
static void to_ciexyz(const libcolour_colour_t* restrict from, libcolour_ciexyz_t* restrict to);
static void to_cielab(const libcolour_colour_t* restrict from, libcolour_cielab_t* restrict to);
static void to_cieluv(const libcolour_colour_t* restrict from, libcolour_cieluv_t* restrict to);
static void to_cielchuv(const libcolour_colour_t* restrict from, libcolour_cielchuv_t* restrict to);
static void to_yiq(const libcolour_colour_t* restrict from, libcolour_yiq_t* restrict to);
static void to_ydbdr(const libcolour_colour_t* restrict from, libcolour_ydbdr_t* restrict to);
static void to_yuv(const libcolour_colour_t* restrict from, libcolour_yuv_t* restrict to);
static void to_ypbpr(const libcolour_colour_t* restrict from, libcolour_ypbpr_t* restrict to);
static void to_ycgco(const libcolour_colour_t* restrict from, libcolour_ycgco_t* restrict to);
static void to_cie1960ucs(const libcolour_colour_t* restrict from, libcolour_cie1960ucs_t* restrict to);
static void to_cieuvw(const libcolour_colour_t* restrict from, libcolour_cieuvw_t* restrict to);


static void ciexyz_to_rgb(const libcolour_ciexyz_t* restrict from, libcolour_rgb_t* restrict to)
{
  double X = from->X, Y = from->Y, Z = from->Z;
  to->R = to->Minv[0][0] * X + to->Minv[0][1] * Y + to->Minv[0][2] * Z;
  to->G = to->Minv[1][0] * X + to->Minv[1][1] * Y + to->Minv[1][2] * Z;
  to->B = to->Minv[2][0] * X + to->Minv[2][1] * Y + to->Minv[2][2] * Z;
}

static void to_rgb(const libcolour_colour_t* restrict from, libcolour_rgb_t* restrict to)
{
  int have_gamma = 0, with_gamma = to->with_gamma;
  libcolour_ciexyz_t tmp;
  switch (from->model) {
  case LIBCOLOUR_CIEXYZ:
    ciexyz_to_rgb(&from->ciexyz, to);
    break;
  case LIBCOLOUR_RGB:
    if (!memcmp(from->rgb.M, to->M, sizeof(double[3][3]))) {
      have_gamma = from->rgb.with_gamma;
      *to = from->rgb;
      break;
    }
    /* fall through */
  default:
    tmp.model = LIBCOLOUR_CIEXYZ;
    to_ciexyz(from, &tmp);
    ciexyz_to_rgb(&tmp, to);
    break;
  }

  if (have_gamma != with_gamma) {
    if (with_gamma) {
      switch (to->encoding_type) {
      case LIBCOLOUR_ENCODING_TYPE_LINEAR:
	break;
      case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
	to->R = pow(to->R, 1 / to->gamma);
	to->G = pow(to->G, 1 / to->gamma);
	to->B = pow(to->B, 1 / to->gamma);
	break;
      case LIBCOLOUR_ENCODING_TYPE_REGULAR:
	to->R <= to->transition ? to->slope * to->R : (1 + to->offset) * pow(to->R, 1 / to->gamma) - to->offset;
	to->G <= to->transition ? to->slope * to->G : (1 + to->offset) * pow(to->G, 1 / to->gamma) - to->offset;
	to->B <= to->transition ? to->slope * to->B : (1 + to->offset) * pow(to->B, 1 / to->gamma) - to->offset;
	break;
      case LIBCOLOUR_ENCODING_TYPE_CUSTOM:
	to->R = (to->to_encoded_red)(to->R);
	to->G = (to->to_encoded_green)(to->G);
	to->B = (to->to_encoded_blue)(to->B);
	break;
      default:
	fprintf(stderr, "libcolour: invalid encoding type\n");
	abort();
      }
    } else {
      switch (to->encoding_type) {
      case LIBCOLOUR_ENCODING_TYPE_LINEAR:
	break;
      case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
	to->R = pow(to->R, to->gamma);
	to->G = pow(to->G, to->gamma);
	to->B = pow(to->B, to->gamma);
	break;
      case LIBCOLOUR_ENCODING_TYPE_REGULAR:
	to->R <= to->transitioninv ? to->R * to->slope : pow((to->R + to->offset) / (1 + to->offset), to->gamma);
	to->G <= to->transitioninv ? to->G * to->slope : pow((to->G + to->offset) / (1 + to->offset), to->gamma);
	to->B <= to->transitioninv ? to->B * to->slope : pow((to->B + to->offset) / (1 + to->offset), to->gamma);
	break;
      case LIBCOLOUR_ENCODING_TYPE_CUSTOM:
	to->R = (to->to_decoded_red)(to->R);
	to->G = (to->to_decoded_green)(to->G);
	to->B = (to->to_decoded_blue)(to->B);
	break;
      default:
	fprintf(stderr, "libcolour: invalid encoding type\n");
	abort();
      }
    }
  }
}


static void ciexyz_to_srgb(const libcolour_ciexyz_t* restrict from, libcolour_srgb_t* restrict to)
{
  double X = from->X, Y = from->Y, Z = from->Z;
  to->R =  3.240450 * X + -1.537140 * Y + -0.4985320 * Z;
  to->G = -0.969266 * X +  1.876010 * Y +  0.0415561 * Z;
  to->B = 0.0556434 * X + -0.204026 * Y +  1.0572300 * Z;
}

static inline double srgb_encode(double x)
{
  return x <= 0.0031306684425217108 ? 12.92 * x : 1.055 * pow(x, 1 / 2.4) - 0.055;
}

static inline double srgb_decode(double x)
{
  return x <= 0.040448236277380506 ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
}

static void srgb_to_srgb(const libcolour_srgb_t* restrict from, libcolour_srgb_t* restrict to)
{
  if (from->with_gamma == to->with_gamma) {
    *to = *from;
  } else if (to->with_gamma) {
    to->R = srgb_encode(from->R);
    to->G = srgb_encode(from->G);
    to->B = srgb_encode(from->B);
  } else {
    to->R = srgb_decode(from->R);
    to->G = srgb_decode(from->G);
    to->B = srgb_decode(from->B);
  }
}

static void yiq_to_srgb(const libcolour_yiq_t* restrict from, libcolour_srgb_t* restrict to)
{
  double Y = from->Y, I = from->I, Q = from->Q;
  to->R = Y + I * 0.956 + Q * 0.621;
  to->G = Y - I * 0.272 - Q * 0.647;
  to->B = Y - I * 1.106 + Q * 1.703;
}

static void ydbdr_to_srgb(const libcolour_ydbdr_t* restrict from, libcolour_srgb_t* restrict to)
{
  double Y = from->Y, Db = from->Db, Dr = from->Dr;
  to->R = Y + Db * 0.000092303716148 - Dr * 0.525912630661865;
  to->G = Y - Db * 0.129132898890509 + Dr * 0.267899328207599;
  to->B = Y + Db * 0.664679059978955 - Dr * 0.000079202543533;
}

static void ypbpr_to_srgb(const libcolour_ypbpr_t* restrict from, libcolour_srgb_t* restrict to)
{
  double Y = from->Y, Pb = from->Pb, Pr = from->Pr;
  to->R = Pr + Y;
  to->B = Pb + Y;
  to->G = (Y - to->R * 0.2126 - to->B * 0.0722) / 0.7152;
}

static void ycgco_to_srgb(const libcolour_ycgco_t* restrict from, libcolour_srgb_t* restrict to)
{
  double Y = from->Y, Cg = from->Cg, Co = from->Co;
  to->R = Y - Cg + Co;
  to->G = Y + Cg;
  to->B = Y - Cg - Co;
}

static void other_to_srgb(const libcolour_colour_t* restrict from, libcolour_srgb_t* restrict to)
{
  libcolour_ciexyz_t tmp;
  tmp.model = LIBCOLOUR_CIEXYZ;
  to_ciexyz(from, &tmp);
  ciexyz_to_srgb(&tmp, to);
}

static void to_srgb(const libcolour_colour_t* restrict from, libcolour_srgb_t* restrict to)
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
  case LIBCOLOUR_YCGCO:
    ycgco_to_srgb(&from->ycgco, to);
    break;
  default:
    other_to_srgb(from, to);
    break;
  }
  if (to->with_gamma) {
    tmp = *to;
    tmp.with_gamma = 0;
    srgb_to_srgb(&tmp, to);
  }
}


static void ciexyz_to_ciexyy(const libcolour_ciexyz_t* restrict from, libcolour_ciexyy_t* restrict to)
{
  double X = from->X, Y = from->Y, Z = from->Z;
  double s = X + Y + Z;
  to->x = X / s;
  to->y = Y / s;
  to->Y = Y;
  if (WASDIV0(to->x) || WASDIV0(to->y))
    to->x = to->y = 0;
}

static void other_to_ciexyy(const libcolour_colour_t* restrict from, libcolour_ciexyy_t* restrict to)
{
  libcolour_ciexyz_t tmp;
  tmp.model = LIBCOLOUR_CIEXYZ;
  to_ciexyz(from, &tmp);
  ciexyz_to_ciexyy(&tmp, to);
}

static void srgb_to_ciexyy(const libcolour_srgb_t* restrict from, libcolour_ciexyy_t* restrict to)
{
  libcolour_srgb_t tmp;
  tmp.model = LIBCOLOUR_SRGB;
  if (from->with_gamma) {
    tmp.with_gamma = 0;
    to_srgb((const libcolour_colour_t*)from, &tmp);
  } else {
    tmp = *from;
  }
  if (tmp.R == 0 && tmp.G == 0 && tmp.B == 0) {
    to->x = 0.31272660439158;
    to->y = 0.32902315240275;
    to->Y = 0;
  } else {
    other_to_ciexyy((const libcolour_colour_t*)&tmp, to);
  }
}

static void to_ciexyy(const libcolour_colour_t* restrict from, libcolour_ciexyy_t* restrict to)
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


static void rgb_to_ciexyz(const libcolour_rgb_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  libcolour_rgb_t tmp;
  double R, G, B;
  if (from->with_gamma) {
    tmp = *from;
    tmp.with_gamma = 0;
    to_rgb((const libcolour_colour_t*)from, &tmp);
    R = tmp.R, G = tmp.G, B = tmp.B;
  } else {
    R = from->R, G = from->G, B = from->B;
  }
  to->X = from->M[0][0] * R + from->M[0][1] * G + from->M[0][2] * B;
  to->Y = from->M[1][0] * R + from->M[1][1] * G + from->M[1][2] * B;
  to->Z = from->M[2][0] * R + from->M[2][1] * G + from->M[2][2] * B;
}

static void srgb_to_ciexyz(const libcolour_srgb_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  libcolour_srgb_t tmp;
  double R, G, B;
  if (from->with_gamma) {
    tmp.model = LIBCOLOUR_SRGB;
    tmp.with_gamma = 0;
    to_srgb((const libcolour_colour_t*)from, &tmp);
    R = tmp.R, G = tmp.G, B = tmp.B;
  } else {
    R = from->R, G = from->G, B = from->B;
  }
  to->X = 0.4124564 * R + 0.3575761 * G + 0.1804375 * B;
  to->Y = 0.2126729 * R + 0.7151522 * G + 0.0721750 * B;
  to->Z = 0.0193339 * R + 0.1191920 * G + 0.9503041 * B;
}

static void ciexyy_to_ciexyz(const libcolour_ciexyy_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  double x = from->x, y = from->y, Y = from->Y;
  double Yy = Y / y;
  if (WASDIV0(Yy)) {
    to->X = to->Y = to->Z = Y;
  } else {
    to->X = x * Yy;
    to->Y = Y;
    to->Z = (1 - x - y) * Yy;
  }
}

static inline double cielab_finv(double t)
{
  return (t > 6. / 29.) ? t * t * t : (t - 4. / 29.) * 108 / 841;
}

static void cielab_to_ciexyz(const libcolour_cielab_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  double L = from->L, a = from->a, b = from->b;
  to->Y = (L + 16) / 116;
  to->X = to->Y + a / 500;
  to->Z = to->Y - b / 200;
  to->X = cielab_finv(to->X) * 0.95047;
  to->Y = cielab_finv(to->Y) * 1.08883;
  to->Z = cielab_finv(to->Z);
}

static void cieluv_to_ciexyz(const libcolour_cieluv_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  double X = from->white.X, Y = from->white.Y, L = from->L, L13 = from->L * 13;
  double t = X + 15 * Y + 3 * from->white.Z;
  double u = from->u / L13 + 4 * X / t;
  double v = from->v / L13 + 9 * Y / t;
  if (L <= 8) {
    Y *= L * 27 / 24389;
  } else {
    L = (L + 16) / 116;
    Y *= L * L * L;
  }
  to->X = 2.25 * Y * u / v;
  to->Y = Y;
  to->Z = Y * (3 / v - 0.75 * u / v - 5);
}

static void cielchuv_to_ciexyz(const libcolour_cielchuv_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  libcolour_cieluv_t tmp;
  tmp.model = LIBCOLOUR_CIELUV;
  tmp.white = from->white;
  to_cieluv((const libcolour_colour_t*)from, &tmp);
  cieluv_to_ciexyz(&tmp, to);
}

static void yuv_to_ciexyz(const libcolour_yuv_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  libcolour_ydbdr_t tmp;
  tmp.model = LIBCOLOUR_YDBDR;
  to_ydbdr((const libcolour_colour_t*)from, &tmp);
  to_ciexyz((const libcolour_colour_t*)&tmp, to);
}

static void cie1960ucs_to_ciexyz(const libcolour_cie1960ucs_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  double u = from->u, v = from->v, Y = from->Y;
  to->X = 1.5 * Y * u / v;
  to->Y = Y;
  to->Z = (4 * Y - Y * u - 10 * Y * v) / (2 * v);
}

static void cieuvw_to_ciexyz(const libcolour_cieuvw_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  libcolour_cie1960ucs_t tmp;
  tmp.model = LIBCOLOUR_CIE1960UCS;
  to_cie1960ucs((const libcolour_colour_t*)from, &tmp);
  cie1960ucs_to_ciexyz(&tmp, to);
}

static void other_to_ciexyz(const libcolour_colour_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  libcolour_srgb_t tmp;
  tmp.model = LIBCOLOUR_SRGB;
  tmp.with_gamma = 0;
  to_srgb(from, &tmp);
  srgb_to_ciexyz(&tmp, to);
}

static void to_ciexyz(const libcolour_colour_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  switch (from->model) {
  case LIBCOLOUR_RGB:
    rgb_to_ciexyz(&from->rgb, to);
    return;
  case LIBCOLOUR_SRGB:
    srgb_to_ciexyz(&from->srgb, to);
    return;
  case LIBCOLOUR_CIEXYY:
    ciexyy_to_ciexyz(&from->ciexyy, to);
    return;
  case LIBCOLOUR_CIEXYZ:
    *to = from->ciexyz;
    return;
  case LIBCOLOUR_CIELAB:
    cielab_to_ciexyz(&from->cielab, to);
    return;
  case LIBCOLOUR_CIELUV:
    cieluv_to_ciexyz(&from->cieluv, to);
    return;
  case LIBCOLOUR_CIELCHUV:
    cielchuv_to_ciexyz(&from->cielchuv, to);
    return;
  case LIBCOLOUR_YUV:
    yuv_to_ciexyz(&from->yuv, to);
    return;
  case LIBCOLOUR_CIE1960UCS:
    cie1960ucs_to_ciexyz(&from->cie1960ucs, to);
    return;
  case LIBCOLOUR_CIEUVW:
    cieuvw_to_ciexyz(&from->cieuvw, to);
    return;
  default:
    other_to_ciexyz(from, to);
    return;
  }
}


static inline double cielab_f(double t)
{
  return (t > 216. / 24389.) ? cbrt(t) : t * 841. / 108. + 4. / 29.;
}

static void ciexyz_to_cielab(const libcolour_ciexyz_t* restrict from, libcolour_cielab_t* restrict to)
{
  double X = from->X, Y = from->Y, Z = from->Z;
  X /= 0.95047;
  Y /= 1.08883;
  Y = cielab_f(Y);
  to->L = 116 * Y - 16;
  to->a = 500 * (cielab_f(X) - Y);
  to->b = 200 * (Y - cielab_f(Z));
}

static void other_to_cielab(const libcolour_colour_t* restrict from, libcolour_cielab_t* restrict to)
{
  libcolour_ciexyz_t tmp;
  tmp.model = LIBCOLOUR_CIEXYZ;
  to_ciexyz(from, &tmp);
  ciexyz_to_cielab(&tmp, to);
}

static void to_cielab(const libcolour_colour_t* restrict from, libcolour_cielab_t* restrict to)
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


static void ciexyz_to_cieluv(const libcolour_ciexyz_t* restrict from, libcolour_cieluv_t* restrict to)
{
  double t, u, v, y, y2;
  t = to->white.X + 15 * to->white.Y + 3 * to->white.Z;
  u = 4 * to->white.X / t;
  v = 9 * to->white.Y / t;
  t = from->X + 15 * from->Y + 3 * from->Z;
  u = 4 * from->X / t - u;
  v = 9 * from->Y / t - v;
  y = from->Y / to->white.Y;
  y2 = y * 24389;
  y = y2 <= 216 ? y2 / 27 : cbrt(y) * 116 - 16;
  to->L = y;
  y *= 13;
  to->u = u * y;
  to->v = v * y;
}

static void cielchuv_to_cieluv(const libcolour_cielchuv_t* restrict from, libcolour_cieluv_t* restrict to)
{
  libcolour_ciexyz_t tmp;
  libcolour_cielchuv_t tmp2;
  double L, C, h;
  if (to->white.X != from->white.X || to->white.Y != from->white.Y || to->white.Z != from->white.Z) {
    tmp.model = LIBCOLOUR_CIEXYZ;
    tmp2.model = LIBCOLOUR_CIELCHUV;
    tmp2.white = to->white;
    to_ciexyz((const libcolour_colour_t*)from, &tmp);
    to_cielchuv((const libcolour_colour_t*)&tmp, &tmp2);
    L = tmp2.L, C = tmp2.C, h = tmp2.h;
  } else {
    L = from->L, C = from->C, h = from->h;
  }
  to->L = L;
  to->u = C * cos(h);
  to->v = C * sin(h);
}

static void other_to_cieluv(const libcolour_colour_t* restrict from, libcolour_cieluv_t* restrict to)
{
  libcolour_ciexyz_t tmp;
  tmp.model = LIBCOLOUR_CIEXYZ;
  to_ciexyz(from, &tmp);
  ciexyz_to_cieluv(&tmp, to);
}

static void to_cieluv(const libcolour_colour_t* restrict from, libcolour_cieluv_t* restrict to)
{
  switch (from->model) {
  case LIBCOLOUR_CIEXYZ:
    ciexyz_to_cieluv(&from->ciexyz, to);
    return;
  case LIBCOLOUR_CIELCHUV:
    cielchuv_to_cieluv(&from->cielchuv, to);
    return;
  case LIBCOLOUR_CIELUV:
    if (to->white.X == from->cieluv.white.X &&
	to->white.Y == from->cieluv.white.Y &&
	to->white.Z == from->cieluv.white.Z) {
      *to = from->cieluv;
      return;
    }
    /* fall through */
  default:
    other_to_cieluv(from, to);
    return;
  }
}


static void cieluv_to_cielchuv(const libcolour_cieluv_t* restrict from, libcolour_cielchuv_t* restrict to)
{
  libcolour_cieluv_t tmp;
  double L, u, v;
  if (to->white.X != from->white.X || to->white.Y != from->white.Y || to->white.Z != from->white.Z) {
    tmp.model = LIBCOLOUR_CIELUV;
    tmp.white = to->white;
    to_cieluv((const libcolour_colour_t*)from, &tmp);
    L = tmp.L, u = tmp.u, v = tmp.v;
  } else {
    L = from->L, u = from->u, v = from->v;
  }
  to->L = L;
  to->C = sqrt(u * u + v * v);
  to->h = atan2(v, u);
}

static void other_to_cielchuv(const libcolour_colour_t* restrict from, libcolour_cielchuv_t* restrict to)
{
  libcolour_cieluv_t tmp;
  tmp.model = LIBCOLOUR_CIELUV;
  tmp.white = to->white;
  to_cieluv(from, &tmp);
  cieluv_to_cielchuv(&tmp, to);
}

static void to_cielchuv(const libcolour_colour_t* restrict from, libcolour_cielchuv_t* restrict to)
{
  switch (from->model) {
  case LIBCOLOUR_CIELUV:
    cieluv_to_cielchuv(&from->cieluv, to);
    return;
  case LIBCOLOUR_CIELCHUV:
    if (to->white.X == from->cielchuv.white.X &&
	to->white.Y == from->cielchuv.white.Y &&
	to->white.Z == from->cielchuv.white.Z) {
      *to = from->cielchuv;
      return;
    }
    /* fall through */
  default:
    other_to_cielchuv(from, to);
    return;
  }
}


static void to_yiq(const libcolour_colour_t* restrict from, libcolour_yiq_t* restrict to)
{
  /* TODO to_yiq: add direct conversion from and to CIE XYZ and use it as the default intermediary */
  double r, g, b;
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YIQ:
    *to = from->yiq;
    return;
  default:
    tmp.model = LIBCOLOUR_SRGB;
    tmp.srgb.with_gamma = 0;
    to_srgb(from, &tmp.srgb);
    /* fall through */
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
    r = tmp.srgb.R;
    g = tmp.srgb.G;
    b = tmp.srgb.B;
    to->Y = r * 0.29893602129377540 + g * 0.5870430744511212 + b * 0.11402090425510336;
    to->I = r * 0.59594574307079930 - g * 0.2743886357457892 - b * 0.32155710732501010;
    to->Q = r * 0.21149734030682846 - g * 0.5229106903029739 + b * 0.31141334999614540;
    return;
  }
}


static void to_ydbdr(const libcolour_colour_t* restrict from, libcolour_ydbdr_t* restrict to)
{
  /* TODO to_ydbdr: add direct conversion from and to CIE XYZ and use it as the default intermediary */
  double r, g, b;
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YDBDR:
    *to = from->ydbdr;
    return;
  case LIBCOLOUR_YUV:
    to->Y  =  tmp.yuv.Y;
    to->Db =  tmp.yuv.U * 3.069;
    to->Dr = -tmp.yuv.V * 2.169;
    return;
  default:
    tmp.model = LIBCOLOUR_SRGB;
    tmp.srgb.with_gamma = 0;
    to_srgb(from, &tmp.srgb);
    /* fall through */
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
    r = tmp.srgb.R;
    g = tmp.srgb.G;
    b = tmp.srgb.B;
    to->Y  =  r * 0.299 + g * 0.587 + b * 0.114;
    to->Db = -r * 0.450 - g * 0.883 + b * 1.333;
    to->Dr = -r * 1.333 + g * 1.116 + b * 0.217;
    return;
  }
}


static void to_yuv(const libcolour_colour_t* restrict from, libcolour_yuv_t* restrict to)
{
  /* TODO to_yuv: add direct conversion from and to CIE XYZ and use it as the default intermediary */
  /* TODO to_yub: add direct conversion from and to sRGB */
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YUV:
    *to = from->yuv;
    return;
  default:
    tmp.model = LIBCOLOUR_YDBDR;
    to_ydbdr(from, &tmp.ydbdr);
    /* fall through */
  case LIBCOLOUR_YDBDR:
    to->Y =  tmp.ydbdr.Y;
    to->U =  tmp.ydbdr.Db / 3.069;
    to->V = -tmp.ydbdr.Dr / 2.169;
    return;
  }
}


static void to_ypbpr(const libcolour_colour_t* restrict from, libcolour_ypbpr_t* restrict to)
{
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YPBPR:
    *to = from->ypbpr;
    return;
  default:
    tmp.model = LIBCOLOUR_SRGB;
    tmp.srgb.with_gamma = 0;
    to_srgb(from, &tmp.srgb);
    /* fall through */
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
    to->Y  = tmp.srgb.R * 0.2126 + tmp.srgb.G * 0.7152 + tmp.srgb.B * 0.0722;
    to->Pb = tmp.srgb.B - to->Y;
    to->Pr = tmp.srgb.R - to->Y;
    return;
  }
}


static void to_ycgco(const libcolour_colour_t* restrict from, libcolour_ycgco_t* restrict to)
{
  /* TODO to_ycgco: add direct conversion from and to CIE XYZ and use it as the default intermediary */
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YCGCO:
    *to = from->ycgco;
    return;
  default:
    tmp.model = LIBCOLOUR_SRGB;
    tmp.srgb.with_gamma = 0;
    to_srgb(from, &tmp.srgb);
    /* fall through */
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
    to->Y  =  tmp.srgb.R / 4 + tmp.srgb.G / 2 + tmp.srgb.B / 4;
    to->Cg = -tmp.srgb.R / 4 + tmp.srgb.G / 2 - tmp.srgb.B / 4;
    to->Co =  tmp.srgb.R / 2 - tmp.srgb.B / 2;
    return;
  }
}


static void to_cie1960ucs(const libcolour_colour_t* restrict from, libcolour_cie1960ucs_t* restrict to)
{
  libcolour_colour_t tmp = *from;
  double u, v, w, y;
  switch (from->model) {
  case LIBCOLOUR_CIE1960UCS:
    *to = from->cie1960ucs;
    return;
  case LIBCOLOUR_CIEUVW:
    u = from->cieuvw.U, v = from->cieuvw.V, w = from->cieuvw.W;
    y = (w + 17) / 25;
    y *= y * y;
    w *= 13;
    to->u = u / w + from->cieuvw.u0;
    to->v = v / w + from->cieuvw.v0;
    to->Y = y;
    return;
  default:
    tmp.model = LIBCOLOUR_CIEXYZ;
    to_ciexyz(from, &tmp.ciexyz);
    /* fall through */
  case LIBCOLOUR_CIEXYZ:
    w = tmp.ciexyz.X + 15 * tmp.ciexyz.Y + 3 * tmp.ciexyz.Z;
    to->u = 4 * tmp.ciexyz.X / w;
    to->v = 6 * tmp.ciexyz.Y / w;
    to->Y = tmp.ciexyz.Y;
    return;
  }
}


static void to_cieuvw(const libcolour_colour_t* restrict from, libcolour_cieuvw_t* restrict to)
{
  double U, V, W, w;
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_CIEUVW:
    U = from->cieuvw.U, V = from->cieuvw.V, W = from->cieuvw.W;
    w = W * 13;
    U += w * (from->cieuvw.u0 - to->u0);
    V += w * (from->cieuvw.v0 - to->v0);
    to->U = U, to->V = V, to->W = W;
    return;
  default:
    tmp.model = LIBCOLOUR_CIE1960UCS;
    to_cie1960ucs(from, &tmp.cie1960ucs);
    /* fall through */
  case LIBCOLOUR_CIE1960UCS:
    U = tmp.cie1960ucs.u, V = tmp.cie1960ucs.v, W = tmp.cie1960ucs.Y;
    W = 25 * cbrt(W) - 17;
    w = W * 13;
    to->U = w * (U - to->u0);
    to->V = w * (V - to->v0);
    to->W = W;
    return;
  }
}



int libcolour_convert(const libcolour_colour_t* restrict from, libcolour_colour_t* restrict to)
{
#define X(C, T) 1 +
  if (from->model < 0 || from->model > LIBCOLOUR_LIST_MODELS 0) {
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

