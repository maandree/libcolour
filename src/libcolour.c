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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



#define MARSHAL_VERSION  0



#define WASDIV0(x)  (isinf(x) || isnan(x))


static void to_rgb(const libcolour_colour_t* restrict from, libcolour_rgb_t* restrict to);
static void to_srgb(const libcolour_colour_t* restrict from, libcolour_srgb_t* restrict to);
static void to_ciexyy(const libcolour_colour_t* restrict from, libcolour_ciexyy_t* restrict to);
static void to_ciexyz(const libcolour_colour_t* restrict from, libcolour_ciexyz_t* restrict to);
static void to_cielab(const libcolour_colour_t* restrict from, libcolour_cielab_t* restrict to);
static void to_cieluv(const libcolour_colour_t* restrict from, libcolour_cieluv_t* restrict to);
static void to_cielch(const libcolour_colour_t* restrict from, libcolour_cielch_t* restrict to);
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

static void srgb_to_srgb(const libcolour_srgb_t* restrict from, libcolour_srgb_t* restrict to)
{
  if (from->with_gamma == to->with_gamma) {
    *to = *from;
  } else if (to->with_gamma) {
    to->R = libcolour_srgb_encode(from->R);
    to->G = libcolour_srgb_encode(from->G);
    to->B = libcolour_srgb_encode(from->B);
  } else {
    to->R = libcolour_srgb_decode(from->R);
    to->G = libcolour_srgb_decode(from->G);
    to->B = libcolour_srgb_decode(from->B);
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

static inline double cielab_finv(double x)
{
  double x3 = x * x * x;
  return (x3 > 0.00885642) ? x3 : (x - 0.1379310) / (7.78 + 703 / 99900);
}

static void cielab_to_ciexyz(const libcolour_cielab_t* restrict from, libcolour_ciexyz_t* restrict to)
{
  double L = from->L, a = from->a, b = from->b;
  to->Y = (L + 16) / 116;
  to->X = to->Y + a / 500;
  to->Z = to->Y - b / 300;
  to->X = cielab_finv(to->X) * 0.95047;
  to->Y = cielab_finv(to->Y);
  to->Z = cielab_finv(to->Z) * 1.08883;
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

static void cielch_to_ciexyz(const libcolour_cielch_t* restrict from, libcolour_ciexyz_t* restrict to)
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
  case LIBCOLOUR_CIELCH:
    cielch_to_ciexyz(&from->cielch, to);
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


static inline double cielab_f(double x)
{
  return (x > 0.00885642) ? cbrt(x) : x * (7.78 + 703 / 99900) + 0.1379310;
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
  t = from->X + 15 * from->Y + 4 * from->Z;
  u = 4 * from->X / t - u;
  v = 9 * from->Y / t - v;
  y = from->Y / to->white.Y;
  y2 = y * 24389;
  y = y2 <= 216 ? y2 / 27 : cbrt(y) * 116 - 16;
  to->L = y;
  y  *= 13;
  to->u = u * y;
  to->v = v * y;
}

static void cielch_to_cieluv(const libcolour_cielch_t* restrict from, libcolour_cieluv_t* restrict to)
{
  libcolour_ciexyz_t tmp;
  libcolour_cielch_t tmp2;
  double L, C, h;
  if (to->white.X != from->white.X || to->white.Y != from->white.Y || to->white.Z != from->white.Z) {
    tmp.model = LIBCOLOUR_CIEXYZ;
    tmp2.model = LIBCOLOUR_CIELCH;
    tmp2.white = to->white;
    to_ciexyz((const libcolour_colour_t*)from, &tmp);
    to_cielch((const libcolour_colour_t*)&tmp, &tmp2);
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
  case LIBCOLOUR_CIELCH:
    cielch_to_cieluv(&from->cielch, to);
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


static void cieluv_to_cielch(const libcolour_cieluv_t* restrict from, libcolour_cielch_t* restrict to)
{
  libcolour_cieluv_t tmp;
  double L, u, v;
  if (to->white.X != from->white.X || to->white.Y != from->white.Y || to->white.Z != from->white.Z) {
    tmp.model = LIBCOLOUR_CIELUV;
    tmp.white = to->white;
    to_cieluv((const libcolour_colour_t*)from, &tmp);
    L = tmp.L, u = tmp.u, v = tmp.v;
  } else {
    L = from->L, u = from->u, v = from->u;
  }
  to->L = L;
  to->C = sqrt(u * u + v * v);
  to->h = atan2(v, u);
}

static void other_to_cielch(const libcolour_colour_t* restrict from, libcolour_cielch_t* restrict to)
{
  libcolour_cieluv_t tmp;
  tmp.model = LIBCOLOUR_CIELUV;
  tmp.white = to->white;
  to_cieluv(from, &tmp);
  cieluv_to_cielch(&tmp, to);
}

static void to_cielch(const libcolour_colour_t* restrict from, libcolour_cielch_t* restrict to)
{
  switch (from->model) {
  case LIBCOLOUR_CIELUV:
    cieluv_to_cielch(&from->cieluv, to);
    return;
  case LIBCOLOUR_CIELCH:
    if (to->white.X == from->cielch.white.X &&
	to->white.Y == from->cielch.white.Y &&
	to->white.Z == from->cielch.white.Z) {
      *to = from->cielch;
      return;
    }
    /* fall through */
  default:
    other_to_cielch(from, to);
    return;
  }
}


static void to_yiq(const libcolour_colour_t* restrict from, libcolour_yiq_t* restrict to)
{
  double r, g, b;
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YIQ:
    *to = from->yiq;
    return;
  default:
    tmp.model = LIBCOLOUR_SRGB;
    to_srgb(from, &tmp.srgb);
    /* fall through */
  case LIBCOLOUR_SRGB:
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
    to_srgb(from, &tmp.srgb);
    /* fall through */
  case LIBCOLOUR_SRGB:
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
    to_srgb(from, &tmp.srgb);
    /* fall through */
  case LIBCOLOUR_SRGB:
    to->Y  = tmp.srgb.R * 0.2126 + tmp.srgb.G * 0.7152 + tmp.srgb.B * 0.0722;
    to->Pb = tmp.srgb.B - to->Y;
    to->Pr = tmp.srgb.R - to->Y;
    return;
  }
}


static void to_ycgco(const libcolour_colour_t* restrict from, libcolour_ycgco_t* restrict to)
{
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YCGCO:
    *to = from->ycgco;
    return;
  default:
    tmp.model = LIBCOLOUR_SRGB;
    to_srgb(from, &tmp.srgb);
    /* fall through */
  case LIBCOLOUR_SRGB:
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
    U = from->cie1960ucs.u, V = from->cie1960ucs.v, W = from->cie1960ucs.Y;
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
  if (from->model < 0 || from->model > LIBCOLOUR_CIEUVW) {
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
  case LIBCOLOUR_CIELCH:
    to_cielch(from, &to->cielch);
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


double libcolour_srgb_encode(double x)
{
  return x <= 0.0031306684425217108 ? 12.92 * x : 1.055 * pow(x, 1 / 2.4) - 0.055;
}

double libcolour_srgb_decode(double x)
{
  return x <= 0.040448236277380506 ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
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


static int eliminate(double** M, size_t n, size_t m)
{
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
    for (r2 = r1; --r2;) {
      R2 = M[r2];
      d = R2[r1];
      for (c = 0; c < m; c++)
	R2[c] -= R1[c] * d;
    }
  }
  return 0;
}


int libcolour_proper(libcolour_colour_t* colour)
{
  libcolour_colour_t tmp, r, g, b;
  double m[3][4];
  switch (colour->model) {
  case LIBCOLOUR_CIELUV:
    colour->cieluv.white.model = LIBCOLOUR_CIEXYZ;
    break;
  case LIBCOLOUR_CIELCH:
    colour->cielch.white.model = LIBCOLOUR_CIEXYZ;
    break;
  case LIBCOLOUR_RGB:
    colour->rgb.red.model   = LIBCOLOUR_CIEXYY;
    colour->rgb.green.model = LIBCOLOUR_CIEXYY;
    colour->rgb.blue.model  = LIBCOLOUR_CIEXYY;
    colour->rgb.white.model = LIBCOLOUR_CIEXYY;
    colour->rgb.red.Y   = 1;
    colour->rgb.green.Y = 1;
    colour->rgb.blue.Y  = 1;
    r.model = g.model = b.model = tmp.model = LIBCOLOUR_CIEXYZ;
    libcolour_convert((const libcolour_colour_t*)&colour->rgb.red, &r);
    libcolour_convert((const libcolour_colour_t*)&colour->rgb.green, &g);
    libcolour_convert((const libcolour_colour_t*)&colour->rgb.blue, &b);
    libcolour_convert((const libcolour_colour_t*)&colour->rgb.white, &tmp);
    m[0][0] = r.ciexyz.X, m[0][1] = g.ciexyz.X, m[0][2] = b.ciexyz.X, m[0][3] = tmp.ciexyz.X;
    m[1][0] = r.ciexyz.Y, m[1][1] = g.ciexyz.Y, m[1][2] = b.ciexyz.Y, m[1][3] = tmp.ciexyz.Y;
    m[2][0] = r.ciexyz.Z, m[2][1] = g.ciexyz.Z, m[2][2] = b.ciexyz.Z, m[2][3] = tmp.ciexyz.Z;
    if (eliminate((double**)m, 3, 4))
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

  if (eliminate((double**)M, 3, 6))
    return -1;

  memcpy(M[0], M[0] + 3, 3 * sizeof(double)), M[0][3] = Sr;
  memcpy(M[1], M[1] + 3, 3 * sizeof(double)), M[1][3] = Sg;
  memcpy(M[2], M[2] + 3, 3 * sizeof(double)), M[2][3] = Sb;

  if (eliminate((double**)M, 3, 4))
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

  if (eliminate((double**)M, 3, 6))
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

  if (eliminate((double**)M, 3, 6))
    return -1;

  memcpy(cs->Minv[0], M[0] + 3, 3 * sizeof(double));
  memcpy(cs->Minv[1], M[1] + 3, 3 * sizeof(double));
  memcpy(cs->Minv[2], M[2] + 3, 3 * sizeof(double));

  return 0;
}


static int invert(double **Minv, double **M, size_t n)
{
  double J[3][6];
  size_t i;
  for (i = 0; i < n; i++) {
    memcpy(J[i], M[i], n * sizeof(double));
    memset(J[i] + n, 0, n * sizeof(double));
    J[i][n + i] = 1;
  }
  if (eliminate((double**)J, n, 2 * n))
    return -1;
  for (i = 0; i < n; i++)
    memcpy(M[i], J[i] + n, n * sizeof(double));
  return 0;
}


static void get_transfer_function(libcolour_colour_t* cs)
{
  if (cs->model == LIBCOLOUR_RGB) {
    switch (cs->rgb.colour_space) {
    case LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB_V2: /* TODO L* */
      break;
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_PQ:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_PQ:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_PQ:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_HLG:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_HLG:
    case LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_HLG:
      /* TODO http://www.itu.int/dms_pubrec/itu-r/rec/bt/R-REC-BT.2100-0-201607-I!!PDF-E.pdf */
      break;
    default:
      break;
    }
  }
}


int libcolour_get_rgb_colour_space(libcolour_colour_t* cs_, libcolour_rgb_colour_space_t space)
{
#define XYY(XVALUE, YVALUE)  (libcolour_ciexyy_t){ .model = LIBCOLOUR_CIEXYY, .x = XVALUE, .y = YVALUE, .Y = 1}

  libcolour_rgb_t* cs = &cs_->rgb;

  switch (space) {
  case LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MEASUREMENTS:
    if (get_matrices(cs))
      return -1;
    return 0;

  case LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MATRIX:
    if (invert((double**)(cs->Minv), (double**)(cs->M), 3) || get_primaries(cs))
      return -1;
    return 0;

  case LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_INV_MATRIX:
    if (invert((double**)(cs->M), (double**)(cs->Minv), 3) || get_primaries(cs))
      return -1;
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
    cs->green = XYY(0.2640, 0.7170);
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
  if (get_matrices(cs) || libcolour_proper(cs_))
    return -1;
  get_transfer_function(cs_);
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

