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


typedef enum libcolour_rgb_colour_space {
} libcolour_rgb_colour_space_t;


typedef struct libcolour_rgb {
  enum libcolour_model model;
  double R;
  double G;
  double B;
  int with_gamma;
  int regular_gamma;
  double gamma;
  union libcolour_colour red;
  union libcolour_colour green;
  union libcolour_colour blue;
  union libcolour_colour white;
  double (*to_encoded_red)(double);
  double (*to_decoded_red)(double);
  double (*to_encoded_green)(double);
  double (*to_decoded_green)(double);
  double (*to_encoded_blue)(double);
  double (*to_decoded_blue)(double);
  double M[3][3];
  double Minv[3][3];
  enum libcolour_rgb_colour_space colour_space;
} libcolour_srgb_t;

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
  double n;
} libcolour_cielab_t;

typedef struct libcolour_cieluv {
  enum libcolour_model model;
  double L;
  double u;
  double v;
  union libcolour_colour white;
} libcolour_cieluv_t;

typedef struct libcolour_cielch {
  enum libcolour_model model;
  double L;
  double C;
  double h;
  union libcolour_colour white;
} libcolour_cielch_t;

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



#endif

