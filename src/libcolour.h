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
#ifndef LIBCOLOUR_H
#define LIBCOLOUR_H



union libcolour_colour;


#define LIBCOLOUR_ILLUMINANT_A    (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.44757, .y = 0.40745, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_B    (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.34842, .y = 0.35161, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_C    (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.31006, .y = 0.31616, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_D50  (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.34567, .y = 0.35850, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_D55  (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.33242, .y = 0.34743, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_D65  (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.31271, .y = 0.32902, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_D75  (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.29902, .y = 0.31485, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_E    (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 1. / 3,  .y = 1. / 3,  .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F1   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.31310, .y = 0.33727, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F2   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.37208, .y = 0.37529, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F3   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.40910, .y = 0.39430, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F4   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.44018, .y = 0.40329, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F5   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.31379, .y = 0.34531, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F6   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.37790, .y = 0.38835, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F7   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.31292, .y = 0.32933, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F8   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.34588, .y = 0.35875, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F9   (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.37417, .y = 0.37281, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F10  (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.34609, .y = 0.35986, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F11  (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.38052, .y = 0.37713, .Y = 1}
#define LIBCOLOUR_ILLUMINANT_F12  (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = 0.43695, .y = 0.40441, .Y = 1}


typedef enum libcolour_model {
  LIBCOLOUR_RGB,
  LIBCOLOUR_SRGB,
  LIBCOLOUR_CIEXYY,
  LIBCOLOUR_CIEXYZ,
  LIBCOLOUR_CIELAB,
  LIBCOLOUR_CIELUV,
  LIBCOLOUR_CIELCH,
  LIBCOLOUR_YIQ,
  LIBCOLOUR_YDBDR,
  LIBCOLOUR_YUV,
  LIBCOLOUR_YPBPR,
  LIBCOLOUR_YCGCO,
  LIBCOLOUR_CIE1960UCS,
  LIBCOLOUR_CIEUVW
} libcolour_model_t;

typedef enum libcolour_encoding_type {
  LIBCOLOUR_ENCODING_TYPE_LINEAR,
  LIBCOLOUR_ENCODING_TYPE_SIMPLE,
  LIBCOLOUR_ENCODING_TYPE_REGULAR,
  LIBCOLOUR_ENCODING_TYPE_CUSTOM
} libcolour_encoding_type;


typedef enum libcolour_rgb_colour_space {
  LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MEASUREMENTS,
  LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_MATRIX,
  LIBCOLOUR_RGB_COLOUR_SPACE_CUSTOM_FROM_INV_MATRIX,
  LIBCOLOUR_RGB_COLOUR_SPACE_SRGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_ADOBE_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_APPLE_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_BEST_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_BETA_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_BRUCE_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_CIE_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_COLORMATCH_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_DCI_P3_D65,
  LIBCOLOUR_RGB_COLOUR_SPACE_DCI_P3_THEATER,
  LIBCOLOUR_RGB_COLOUR_SPACE_DON_RGB_4,
  LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB_V2,
  LIBCOLOUR_RGB_COLOUR_SPACE_EKTA_SPACE_PS5,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_601_625_LINE,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_601_525_LINE,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_709,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2020,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_PQ,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_PQ,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_PQ,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_EOTF_HLG,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OOTF_HLG,
  LIBCOLOUR_RGB_COLOUR_SPACE_ITU_R_BT_2100_OETF_HLG,
  LIBCOLOUR_RGB_COLOUR_SPACE_LIGHTROOM_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_NTSC_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_PAL_SECAM_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_PROPHOTO_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_SGI_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_SMPTE_240M_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_SMPTE_C_RGB,
  LIBCOLOUR_RGB_COLOUR_SPACE_WIDE_GAMUT_RGB
} libcolour_rgb_colour_space_t;


typedef struct libcolour_srgb {
  enum libcolour_model model;
  double R;
  double G;
  double B;
  int with_gamma;
} libcolour_srgb_t;

typedef struct libcolour_ciexyy {
  enum libcolour_model model;
  double x;
  double y;
  double Y;
} libcolour_ciexyy_t;

typedef struct libcolour_ciexyz {
  enum libcolour_model model;
  double X;
  double Y;
  double Z;
} libcolour_ciexyz_t;

typedef struct libcolour_cielab {
  enum libcolour_model model;
  double L;
  double a;
  double b;
} libcolour_cielab_t;

typedef struct libcolour_yiq {
  enum libcolour_model model;
  double Y;
  double I;
  double Q;
} libcolour_yiq_t;

typedef struct libcolour_ydbdr {
  enum libcolour_model model;
  double Y;
  double Db;
  double Dr;
} libcolour_ydbdr_t;

typedef struct libcolour_yuv {
  enum libcolour_model model;
  double Y;
  double U;
  double V;
} libcolour_yuv_t;

typedef struct libcolour_ypbpr {
  enum libcolour_model model;
  double Y;
  double Pb;
  double Pr;
} libcolour_ypbpr_t;

typedef struct libcolour_ycgco {
  enum libcolour_model model;
  double Y;
  double Cg;
  double Co;
} libcolour_ycgco_t;

typedef struct libcolour_cie1960ucs {
  enum libcolour_model model;
  double u;
  double v;
  double Y;
} libcolour_cie1960ucs_t;

typedef struct libcolour_cieuvw {
  enum libcolour_model model;
  double U;
  double V;
  double W;
  double u0;
  double v0;
} libcolour_cieuvw_t;

typedef struct libcolour_cieluv {
  enum libcolour_model model;
  double L;
  double u;
  double v;
  struct libcolour_ciexyz white;
} libcolour_cieluv_t;

typedef struct libcolour_cielch {
  enum libcolour_model model;
  double L;
  double C;
  double h;
  struct libcolour_ciexyz white;
} libcolour_cielch_t;

typedef struct libcolour_rgb {
  enum libcolour_model model;
  double R;
  double G;
  double B;
  int with_gamma;
  enum libcolour_encoding_type encoding_type;
  double gamma;
  double offset;
  double slope;
  double transition;
  double transitioninv;
  struct libcolour_ciexyy red;
  struct libcolour_ciexyy green;
  struct libcolour_ciexyy blue;
  struct libcolour_ciexyy white;
  double (*to_encoded_red)(double);
  double (*to_decoded_red)(double);
  double (*to_encoded_green)(double);
  double (*to_decoded_green)(double);
  double (*to_encoded_blue)(double);
  double (*to_decoded_blue)(double);
  double M[3][3];
  double Minv[3][3];
  double white_r;
  double white_g;
  double white_b;
  enum libcolour_rgb_colour_space colour_space;
} libcolour_rgb_t;

/* TODO LMS, L'M'S', ICtCp */
/* TODO generic additive colour space */
/* TODO generic subtractive colour space */


typedef union libcolour_colour {
  enum libcolour_model model;
  struct libcolour_rgb rgb;
  struct libcolour_srgb srgb;
  struct libcolour_ciexyy ciexyy;
  struct libcolour_ciexyz ciexyz;
  struct libcolour_cielab cielab;
  struct libcolour_cieluv cieluv;
  struct libcolour_cielch cielch;
  struct libcolour_yiq yiq;
  struct libcolour_ydbdr ydbdr;
  struct libcolour_yuv yuv;
  struct libcolour_ypbpr ypbpr;
  struct libcolour_ycgco ycgco;
  struct libcolour_cie1960ucs cie1960ucs;
  struct libcolour_cieuvw cieuvw;
} libcolour_colour_t;



int libcolour_convert(const libcolour_colour_t* restrict from, libcolour_colour_t* restrict to);
double libcolour_srgb_encode(double);
double libcolour_srgb_decode(double);
int libcolour_delta_e(const libcolour_colour_t*, const libcolour_colour_t*, double*);
int libcolour_proper(libcolour_colour_t*);
int libcolour_get_rgb_colour_space(libcolour_colour_t*, libcolour_rgb_colour_space_t);
/* TODO (un)marshal */



#endif

