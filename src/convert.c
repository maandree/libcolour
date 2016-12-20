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
#define PI          (3.14159265358979323846)
#define PI2         (2 * PI)



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

#define REGULAR(s, t)     ((t) <= (s)->transition ? (s)->slope * (t) : (1 + (s)->offset) * pow((t), 1 / (s)->gamma) - (s)->offset)
#define INVREGULAR(s, t)  ((t) <= (s)->transitioninv ? (t) / (s)->slope : pow(((t) + (s)->offset) / (1 + (s)->offset), (s)->gamma))

static void rgb_encode(libcolour_rgb_t* restrict colour, const libcolour_rgb_t* restrict space)
{
  double r_sign = 1, g_sign = 1, b_sign = 1;
  switch (space->encoding_type) {
  case LIBCOLOUR_ENCODING_TYPE_LINEAR:
    break;
  case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
  case LIBCOLOUR_ENCODING_TYPE_REGULAR:
    if (colour->R < 0)  r_sign = -1, colour->R = -colour->R;
    if (colour->G < 0)  g_sign = -1, colour->G = -colour->G;
    if (colour->B < 0)  b_sign = -1, colour->B = -colour->B;
    if (space->encoding_type == LIBCOLOUR_ENCODING_TYPE_SIMPLE) {
      colour->R = pow(colour->R, 1 / space->gamma);
      colour->G = pow(colour->G, 1 / space->gamma);
      colour->B = pow(colour->B, 1 / space->gamma);
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
    colour->R = (space->to_encoded_red)(colour->R);
    colour->G = (space->to_encoded_green)(colour->G);
    colour->B = (space->to_encoded_blue)(colour->B);
    break;
  default:
    fprintf(stderr, "libcolour: invalid encoding type\n");
    abort();
  }
}

static void rgb_decode(libcolour_rgb_t* restrict colour, const libcolour_rgb_t* restrict space)
{
  double r_sign = 1, g_sign = 1, b_sign = 1;
  switch (space->encoding_type) {
  case LIBCOLOUR_ENCODING_TYPE_LINEAR:
    break;
  case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
  case LIBCOLOUR_ENCODING_TYPE_REGULAR:
    if (colour->R < 0)  r_sign = -1, colour->R = -colour->R;
    if (colour->G < 0)  g_sign = -1, colour->G = -colour->G;
    if (colour->B < 0)  b_sign = -1, colour->B = -colour->B;
    if (space->encoding_type == LIBCOLOUR_ENCODING_TYPE_SIMPLE) {
      colour->R = pow(colour->R, space->gamma);
      colour->G = pow(colour->G, space->gamma);
      colour->B = pow(colour->B, space->gamma);
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
    colour->R = (space->to_decoded_red)(colour->R);
    colour->G = (space->to_decoded_green)(colour->G);
    colour->B = (space->to_decoded_blue)(colour->B);
    break;
  default:
    fprintf(stderr, "libcolour: invalid encoding type\n");
    abort();
  }
}

#undef REGULAR
#undef INVREGULAR

static int rgb_same_gamma(const libcolour_rgb_t* restrict a, const libcolour_rgb_t* restrict b)
{
  if (a->encoding_type != b->encoding_type)
    return 0;
  switch (a->encoding_type) {
  case LIBCOLOUR_ENCODING_TYPE_SIMPLE:
    return a->gamma == b->gamma;
  case LIBCOLOUR_ENCODING_TYPE_REGULAR:
    return a->gamma      == b->gamma  &&
	   a->offset     == b->offset &&
	   a->slope      == b->slope  &&
	   a->transition == b->transition;
  case LIBCOLOUR_ENCODING_TYPE_CUSTOM:
    return a->to_encoded_red   == b->to_encoded_red   &&
	   a->to_encoded_green == b->to_encoded_green &&
	   a->to_encoded_blue  == b->to_encoded_blue  &&
	   a->to_decoded_red   == b->to_decoded_red   &&
	   a->to_decoded_green == b->to_decoded_green &&
	   a->to_decoded_blue  == b->to_decoded_blue;
  default:
    return 1;
  }
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
      to->R = from->rgb.R;
      to->G = from->rgb.G;
      to->B = from->rgb.B;
      if (have_gamma && with_gamma && !rgb_same_gamma(&from->rgb, to))
	rgb_decode(to, &from->rgb), have_gamma = 0;
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
      rgb_encode(to, to);
    } else {
      rgb_decode(to, &from->rgb);
    }
  }
}


static void ciexyz_to_srgb(const libcolour_ciexyz_t* restrict from, libcolour_srgb_t* restrict to)
{
  double X = from->X, Y = from->Y, Z = from->Z;
#define MULTIPLY(CX, CY, CZ)  ((CX) * X + (CY) * Y + (CZ) * Z)
  to->R = MULTIPLY(3.240446254647737500675930277794, -1.537134761820080575134284117667, -0.498530193022728718155178739835);
  to->G = MULTIPLY(-0.969266606244679751469561779231, 1.876011959788370209167851498933, 0.041556042214430065351304932619);
  to->B = MULTIPLY(0.055643503564352832235773149705, -0.204026179735960239147729566866, 1.057226567722703292062647051353);
#undef MULTIPLY
}

static inline double srgb_encode(double t)
{
  double sign = 1;
  if (t < 0) {
    t = -t;
    sign = -1;
  }
  t = t <= 0.0031306684425217108 ? 12.92 * t : 1.055 * pow(t, 1 / 2.4) - 0.055;
  return t * sign;
}

static inline double srgb_decode(double t)
{
  double sign = 1;
  if (t < 0) {
    t = -t;
    sign = -1;
  }
  t = t <= 0.0031306684425217108 * 12.92 ? t / 12.92 : pow((t + 0.055) / 1.055, 2.4);
  return t * sign;
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
  to->R = Y + I * 0.95629483232089407263032398986979387700557708740234 + Q * 0.62102512544472865396727456754888407886028289794922;
  to->G = Y - I * 0.27212147408397735492968649850809015333652496337891 - Q * 0.64738095351761570928061928498209454119205474853516;
  to->B = Y - I * 1.10698990856712820018969978264067322015762329101562 + Q * 1.70461497549882934343656870623817667365074157714844;
  /* ⎛1    1000 cos 33° / 877                                  1000 sin 33° / 877                               ⎞
     ⎜1    9500 sin 33° / 24067 - 299000 * cos 33° / 514799    -9500 cos 33° / 24067 - 299000 * sin 33° / 514799⎟
     ⎝1    -250 sin 33° / 123                                  250 cos 33° / 123                                ⎠
  */
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

static void yuv_to_srgb(const libcolour_yuv_t* restrict from, libcolour_srgb_t* restrict to)
{
  double Y = from->Y, U = from->U, V = from->V;
  to->R = Y + U * 0.00028328010485821202317155420580263580632163211703 + V * 1.14070449590558520291949662350816652178764343261719;
  to->G = Y - U * 0.39630886669497211727275498560629785060882568359375 - V * 0.58107364288228224857846271333983168005943298339844;
  to->B = Y + U * 2.03990003507541306504435851820744574069976806640625 + V * 0.00017179031692307700847528739718228507626918144524;
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
#define MULTIPLIY(CR, CG, CB)  ((CR) * R + (CG) * G + (CB) * B)
  to->X = MULTIPLIY(0.412457445582367576708548995157, 0.357575865245515878143578447634, 0.180437247826399665973085006954);
  to->Y = MULTIPLIY(0.212673370378408277403536885686, 0.715151730491031756287156895269, 0.072174899130559869164791564344);
  to->Z = MULTIPLIY(0.019333942761673460208893260415, 0.119191955081838593666354597644, 0.950302838552371742508739771438);
#undef MULTIPLIY
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

static void yiq_to_ciexyz(const libcolour_yiq_t* restrict from, libcolour_ciexyz_t* restrict to)
{
#define MULTIPLIY(CY, CI, CQ)  ((CY) * from->Y + (CI) * from->I + (CQ) * from->Q)
  to->X = MULTIPLIY(0.95047055865428309306963683411595411598682403564453, 0.09738463974178063198294807989441324025392532348633, 0.33223466706854809515903070860076695680618286132812);
  to->Y = MULTIPLIY(0.99999999999999988897769753748434595763683319091797, -0.07112658301916767455974621725545148365199565887451, -0.20786968876398304040264974901219829916954040527344);
  to->Z = MULTIPLIY(1.08882873639588373393394249433185905218124389648438, -1.06592139332461721679123911599162966012954711914062, 1.55474471255181900808395312196807935833930969238281);
#undef MULTIPLIY
}

static void ydbdr_to_ciexyz(const libcolour_ydbdr_t* restrict from, libcolour_ciexyz_t* restrict to)
{
#define MULTIPLIY(CY, CDB, CDR)  ((CY) * from->Y + (CDB) * from->Db + (CDR) * from->Dr)
  to->X = MULTIPLIY(0.95047055865428309306963683411595411598682403564453, 0.07379612357298576119646327242662664502859115600586, -0.12113653724874726136384595065464964136481285095215);
  to->Y = MULTIPLIY(0.99999999999999988897769753748434595763683319091797, -0.04435684145428285540813106990754022262990474700928, 0.07973534004202506575431641522300196811556816101074);
  to->Z = MULTIPLIY(1.08882873639588373393394249433185905218124389648438, 0.61625657933494271123464613992837257683277130126953, 0.02168821359337728266192257819966471288353204727173);
#undef MULTIPLIY
}

static void yuv_to_ciexyz(const libcolour_yuv_t* restrict from, libcolour_ciexyz_t* restrict to)
{
#define MULTIPLIY(CY, CU, CV)  ((CY) * from->Y + (CU) * from->U + (CV) * from->V)
  to->X = MULTIPLIY(0.95047055865428309306963683411595411598682403564453, 0.22648030324549334180161963558930438011884689331055, 0.26274514929253273143316960158699657768011093139648);
  to->Y = MULTIPLIY(0.99999999999999988897769753748434595763683319091797, -0.13613114642319409930415474718756740912795066833496, -0.17294595255115238763288232348713790997862815856934);
  to->Z = MULTIPLIY(1.08882873639588373393394249433185905218124389648438, 1.89129144197893928058817891724174842238426208496094, -0.04704173528403532422714761196402832865715026855469);
#undef MULTIPLIY
}

static void ypbpr_to_ciexyz(const libcolour_ypbpr_t* restrict from, libcolour_ciexyz_t* restrict to)
{
#define MULTIPLIY(CY, CPB, CPR)  ((CY) * from->Y + (CPB) * from->Pb + (CPR) * from->Pr)
  to->X = MULTIPLIY(0.95047055865428298204733437160030007362365722656250, 0.14433968424876231217979238863335922360420227050781, 0.30616461986760712399302519770571961998939514160156);
  to->Y = MULTIPLIY(0.99999999999999977795539507496869191527366638183594, -0.00002022802471486617736928792510298080742359161377, 0.00008771894888734421691367515450110659003257751465);
  to->Z = MULTIPLIY(1.08882873639588373393394249433185905218124389648438, 0.93827031735982591165168287261622026562690734863281, -0.01609699914324668607035206946420657914131879806519);
#undef MULTIPLIY
}

static void ycgco_to_ciexyz(const libcolour_ycgco_t* restrict from, libcolour_ciexyz_t* restrict to)
{
#define MULTIPLIY(CY, CCG, CCO)  ((CY) * from->Y + (CCG) * from->Cg + (CCO) * from->Co)
  to->X = MULTIPLIY(0.95047055865428309306963683411595411598682403564453, -0.23531882816325136453805555447615915909409523010254, 0.23202019775596791073546398820326430723071098327637);
  to->Y = MULTIPLIY(0.99999999999999988897769753748434595763683319091797, 0.43030346098206362359661625305307097733020782470703, 0.14049847124784842211653312915586866438388824462891);
  to->Z = MULTIPLIY(1.08882873639588373393394249433185905218124389648438, -0.85044482623220662986796014592982828617095947265625, -0.93096889579069830311652822274481877684593200683594);
#undef MULTIPLIY
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
    tmp2.one_revolution = PI2;
    to_ciexyz((const libcolour_colour_t*)from, &tmp);
    to_cielchuv((const libcolour_colour_t*)&tmp, &tmp2);
    L = tmp2.L, C = tmp2.C, h = tmp2.h;
  } else {
    L = from->L, C = from->C, h = from->h * PI2 / from->one_revolution;
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
  to->h = atan2(v, u) / PI2 * to->one_revolution;
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
  double one_revolution;
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


static void to_yiq(const libcolour_colour_t* restrict from, libcolour_yiq_t* restrict to)
{
  double x, y, z;
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YIQ:
    *to = from->yiq;
    break;
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
    x = tmp.srgb.R;
    y = tmp.srgb.G;
    z = tmp.srgb.B;
    to->Y = x * 0.299 + y * 0.587 + z * 0.114;
    to->I = x * 0.59571613491277464191853141528554260730743408203125  /* (0.877 cos 33°)(1 - 0.299) - (0.492 sin 33°)(-0.299) */
          - y * 0.27445283783925644716106262421817518770694732666016  /* (0.877 cos 33°)(-0.587)    - (0.492 sin 33°)(-0.587) */
          - z * 0.32126329707351808373516632855171337723731994628906; /* (0.877 cos 33°)(-0.114)    - (0.492 sin 33°)(1 - 0.114) */
    to->Q = x * 0.21145640212011795888713550084503367543220520019531  /* (0.877 sin 33°)(1 - 0.299) + (0.492 cos 33°)(-0.299) */
          - y * 0.52259104529161115593183239980135113000869750976562  /* (0.877 sin 33°)(-0.587)    + (0.492 cos 33°)(-0.587) */
          + z * 0.31113464317149330806699936147197149693965911865234; /* (0.877 sin 33°)(-0.114)    + (0.492 cos 33°)(1 - 0.114) */
    break;
  default:
    tmp.model = LIBCOLOUR_CIEXYZ;
    to_ciexyz(from, &tmp.ciexyz);
    /* fall through */
  case LIBCOLOUR_CIEXYZ:
    x = tmp.ciexyz.X;
    y = tmp.ciexyz.Y;
    z = tmp.ciexyz.Z;
#define MULTIPLIY(CX, CY, CZ)  ((CX) * x + (CY) * y + (CZ) * z)
    to->Y = MULTIPLIY(0.40627729168038273499519164033699780702590942382812, 0.61835674212166968910509012857801280915737152099609, -0.00414330221353725880462093300593551248311996459961);
    to->I = MULTIPLIY(2.17852787350219845308174626552499830722808837890625, -1.36502666214454104753883711964590474963188171386719, -0.64803574634025240541745915834326297044754028320312);
    to->Q = MULTIPLIY(1.20905577682138853923277110880007967352867126464844, -1.36890364998339797431015085749095305800437927246094, 0.20180559439597040016778350945969577878713607788086);
#undef MULTIPLIY
    break;
  }
}


static void to_ydbdr(const libcolour_colour_t* restrict from, libcolour_ydbdr_t* restrict to)
{
  double x, y, z;
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YDBDR:
    *to = from->ydbdr;
    break;
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
    x = tmp.srgb.R;
    y = tmp.srgb.G;
    z = tmp.srgb.B;
    to->Y  =  x * 0.299 + y * 0.587 + z * 0.114;
    to->Db = -x * 0.450 - y * 0.883 + z * 1.333;
    to->Dr = -x * 1.333 + y * 1.116 + z * 0.217;
    break;
  case LIBCOLOUR_YUV:
    to->Y  =  tmp.yuv.Y;
    to->Db =  tmp.yuv.U * 3.069;
    to->Dr = -tmp.yuv.V * 2.169;
    break;
  default:
    tmp.model = LIBCOLOUR_CIEXYZ;
    to_ciexyz(from, &tmp.ciexyz);
    /* fall through */
  case LIBCOLOUR_CIEXYZ:
    x = tmp.ciexyz.X;
    y = tmp.ciexyz.Y;
    z = tmp.ciexyz.Z;
#define MULTIPLY(CX, CY, CZ)  ((CX) * x + (CY) * y + (CZ) * z)
    to->Y  = MULTIPLY(0.40627729168038273499519164033699780702590942382812, 0.61835674212166968910509012857801280915737152099609, -0.00414330221353725880462093300593551248311996459961);
    to->Db = MULTIPLY(-0.52816561102614745237815441214479506015777587890625, -1.23677481526212962315014465275453403592109680175781, 1.59692761635924940222253098909277468919754028320312);
    to->Dr = MULTIPLY(-5.38914174974103143966885909321717917919158935546875, 4.09835630362728497999569299281574785709381103515625, 0.94033545560642795013706063400604762136936187744141);
#undef MULTIPLY
    break;
  }
}


static void to_yuv(const libcolour_colour_t* restrict from, libcolour_yuv_t* restrict to)
{
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YUV:
    *to = from->yuv;
    break;
  case LIBCOLOUR_YDBDR:
    to->Y = from->ydbdr.Y;
    to->U = from->ydbdr.Db /  3.069;
    to->V = from->ydbdr.Dr / -2.169;
    break;
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
#define MULTIPLY(CR, CG, CB)  ((CR) * tmp.srgb.R + (CG) * tmp.srgb.G + (CB) * tmp.srgb.B)
    to->Y = MULTIPLY(0.29899999999999998800959133404830936342477798461914, 0.58699999999999996624922005139524117112159729003906, 0.11400000000000000410782519111307919956743717193604);
    to->U = MULTIPLY(-0.14662756598240470062854967636667424812912940979004, -0.28771586836102963635752871596196200698614120483398, 0.43434343434343436474165400795754976570606231689453);
    to->V = MULTIPLY(0.61456892577224520035628074765554629266262054443359, -0.51452282157676354490405401520547457039356231689453, -0.10004610419548178035231700278018251992762088775635);
#undef MULTIPLY
    break;
  default:
    tmp.model = LIBCOLOUR_CIEXYZ;
    to_ciexyz(from, &tmp.ciexyz);
    /* fall through */
  case LIBCOLOUR_CIEXYZ:
#define MULTIPLY(CX, CY, CZ)  ((CX) * tmp.ciexyz.X + (CY) * tmp.ciexyz.Y + (CZ) * tmp.ciexyz.Z)
    to->Y = MULTIPLY(0.40627729168038273499519164033699780702590942382812, 0.61835674212166968910509012857801280915737152099609, -0.00414330221353725880462093300593551248311996459961);
    to->U = MULTIPLY(-0.17209697328971895746718701047939248383045196533203, -0.40298951295605395239718404809536878019571304321289, 0.52034135430408912093014350830344483256340026855469);
    to->V = MULTIPLY(2.48462044709130047692724474472925066947937011718750, -1.88951420176453876997868519538315013051033020019531, -0.43353409663735725798616726933687459677457809448242);
#undef MULTIPLY
    break;
  }
}


static void to_ypbpr(const libcolour_colour_t* restrict from, libcolour_ypbpr_t* restrict to)
{
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YPBPR:
    *to = from->ypbpr;
    break;
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
    to->Y  = tmp.srgb.R * 0.2126 + tmp.srgb.G * 0.7152 + tmp.srgb.B * 0.0722;
    to->Pb = tmp.srgb.B - to->Y;
    to->Pr = tmp.srgb.R - to->Y;
    break;
  default:
    tmp.model = LIBCOLOUR_CIEXYZ;
    to_ciexyz(from, &tmp.ciexyz);
    /* fall through */
  case LIBCOLOUR_CIEXYZ:
#define MULTIPLY(CX, CY, CZ)  ((CX) * tmp.ciexyz.X + (CY) * tmp.ciexyz.Y + (CZ) * tmp.ciexyz.Z)
    to->Y  = MULTIPLY(-0.00028314209073960778378920011277841695118695497513, 1.00019821310075673892470149439759552478790283203125, 0.00006512054470741990286342115723527967929840087891);
    to->Pb = MULTIPLY(0.05592664565509243568275365987574332393705844879150, -1.20422439283671711685030913940863683819770812988281, 1.05716144717799576113748116767965257167816162109375);
    to->Pr = MULTIPLY(3.24072939673847715269516811531502753496170043945312, -2.53733297492083753610359053709544241428375244140625, -0.49859531356743613805804216099204495549201965332031);
#undef MULTIPLY
    break;
  }
}


static void to_ycgco(const libcolour_colour_t* restrict from, libcolour_ycgco_t* restrict to)
{
  libcolour_colour_t tmp = *from;
  switch (from->model) {
  case LIBCOLOUR_YCGCO:
    *to = from->ycgco;
    break;
  case LIBCOLOUR_SRGB:
    if (tmp.srgb.with_gamma) {
      tmp.srgb.with_gamma = 0;
      to_srgb(from, &tmp.srgb);
    }
    to->Y  =  tmp.srgb.R / 4 + tmp.srgb.G / 2 + tmp.srgb.B / 4;
    to->Cg = -tmp.srgb.R / 4 + tmp.srgb.G / 2 - tmp.srgb.B / 4;
    to->Co =  tmp.srgb.R / 2 - tmp.srgb.B / 2;
    break;
  default:
    tmp.model = LIBCOLOUR_CIEXYZ;
    to_ciexyz(from, &tmp.ciexyz);
    /* fall through */
  case LIBCOLOUR_CIEXYZ:
#define MULTIPLY(CX, CY, CZ)  ((CX) * tmp.ciexyz.X + (CY) * tmp.ciexyz.Y + (CZ) * tmp.ciexyz.Z)
    to->Y  = MULTIPLY(0.33938913643068269188063368346774950623512268066406, 0.50271574450517486631895280879689380526542663574219, 0.16045211478220866574417868832824751734733581542969);
    to->Cg = MULTIPLY(-1.30865574267536244335019546269904822111129760742188, 1.37329621528319534284889869013568386435508728027344, -0.11889607256777862120955546743061859160661697387695);
    to->Co = MULTIPLY(1.59240137554169236544510113162687048316001892089844, -0.66655429104206020962664069884340278804302215576172, -0.77787838037271606062006412685150280594825744628906);
#undef MULTIPLY
    break;
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
    break;
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
    break;
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

