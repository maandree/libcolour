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


#include <stddef.h>



#define LIBCOLOUR_ILLUMINANT__(x__, y__)  (libcolour_ciexyy_t){.model = LIBCOLOUR_CIEXYY, .x = x__, .y = y__, .Y = 1}

#define LIBCOLOUR_ILLUMINANT_A    LIBCOLOUR_ILLUMINANT__(0.447573514098910552050369915378, 0.407439444306660847328060981454)
#define LIBCOLOUR_ILLUMINANT_B    LIBCOLOUR_ILLUMINANT__(0.348407693041403399014654951316, 0.351617234807268863594487129376)
#define LIBCOLOUR_ILLUMINANT_C    LIBCOLOUR_ILLUMINANT__(0.310058473730255412803558101587, 0.316149707523236456196968902077)
#define LIBCOLOUR_ILLUMINANT_D50  LIBCOLOUR_ILLUMINANT__(0.345668037029273123028616510055, 0.358496838937619077825047497754)
#define LIBCOLOUR_ILLUMINANT_D55  LIBCOLOUR_ILLUMINANT__(0.332424102468830251488896010414, 0.347428039087666229445261478759)
#define LIBCOLOUR_ILLUMINANT_D65  LIBCOLOUR_ILLUMINANT__(0.312726871026564878786047074755, 0.329023206641284038376227272238)
#define LIBCOLOUR_ILLUMINANT_D75  LIBCOLOUR_ILLUMINANT__(0.299022300412497055166483050925, 0.314852737888341893679466920730)
#define LIBCOLOUR_ILLUMINANT_E    LIBCOLOUR_ILLUMINANT__(1. / 3, 1. / 3)
#define LIBCOLOUR_ILLUMINANT_F1   LIBCOLOUR_ILLUMINANT__(0.313062433035651010992950205036, 0.337106477918307445573731229160)
#define LIBCOLOUR_ILLUMINANT_F2   LIBCOLOUR_ILLUMINANT__(0.372068154452825539113547392844, 0.375122558203110079144693145281)
#define LIBCOLOUR_ILLUMINANT_F3   LIBCOLOUR_ILLUMINANT__(0.409090035308107391465171076561, 0.394117134255365986206243178458)
#define LIBCOLOUR_ILLUMINANT_F4   LIBCOLOUR_ILLUMINANT__(0.440181095827666568620628595454, 0.403090691158138336724903183494)
#define LIBCOLOUR_ILLUMINANT_F5   LIBCOLOUR_ILLUMINANT__(0.313756583095696484075887155996, 0.345160794752101929283583103825)
#define LIBCOLOUR_ILLUMINANT_F6   LIBCOLOUR_ILLUMINANT__(0.377882361062687466279896852939, 0.388192885537868959122675960316)
#define LIBCOLOUR_ILLUMINANT_F7   LIBCOLOUR_ILLUMINANT__(0.312852472915475354753311876266, 0.329174178033567632617462095368)
#define LIBCOLOUR_ILLUMINANT_F8   LIBCOLOUR_ILLUMINANT__(0.345805753550315952971061506105, 0.358617583214377477762724311106)
#define LIBCOLOUR_ILLUMINANT_F9   LIBCOLOUR_ILLUMINANT__(0.374105245592801061160770359493, 0.372672400924498159469067104510)
#define LIBCOLOUR_ILLUMINANT_F10  LIBCOLOUR_ILLUMINANT__(0.346086913993929323751785886998, 0.358751605952200347537939251197)
#define LIBCOLOUR_ILLUMINANT_F11  LIBCOLOUR_ILLUMINANT__(0.380537485483030235577928124258, 0.376915309293930078649026427229)
#define LIBCOLOUR_ILLUMINANT_F12  LIBCOLOUR_ILLUMINANT__(0.437023901312296902954557253906, 0.404214327891585678553809657387)

#define LIBCOLOUR_ILLUMINANT_A_10DEG_OBS    LIBCOLOUR_ILLUMINANT__(0.451173939693730152722395132514, 0.405936604212625562482230634487)
#define LIBCOLOUR_ILLUMINANT_B_10DEG_OBS    LIBCOLOUR_ILLUMINANT__(0.349819801494100579564161535018, 0.352687989927865819250740742063)
#define LIBCOLOUR_ILLUMINANT_C_10DEG_OBS    LIBCOLOUR_ILLUMINANT__(0.310388663270034004248998371622, 0.319050711366790695766582075521)
#define LIBCOLOUR_ILLUMINANT_D50_10DEG_OBS  LIBCOLOUR_ILLUMINANT__(0.347729429961154856698613002663, 0.359522508516545380441442603114)
#define LIBCOLOUR_ILLUMINANT_D55_10DEG_OBS  LIBCOLOUR_ILLUMINANT__(0.334116336430253457745465084372, 0.348766090975953568786849245953)
#define LIBCOLOUR_ILLUMINANT_D65_10DEG_OBS  LIBCOLOUR_ILLUMINANT__(0.313823646938709621689866935412, 0.330998985489933561510156323493)
#define LIBCOLOUR_ILLUMINANT_D75_10DEG_OBS  LIBCOLOUR_ILLUMINANT__(0.299679971345752860223399238748, 0.317403239854836705102769656150)
#define LIBCOLOUR_ILLUMINANT_E_10DEG_OBS    LIBCOLOUR_ILLUMINANT__(1. / 3, 1. / 3)
#define LIBCOLOUR_ILLUMINANT_F1_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.318098801070991199502202562144, 0.335489451474129951602520804954)
#define LIBCOLOUR_ILLUMINANT_F2_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.379274832262508854174853922814, 0.367227934400669309145115448700)
#define LIBCOLOUR_ILLUMINANT_F3_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.417644682102624287267644831445, 0.383124504918675723441623404142)
#define LIBCOLOUR_ILLUMINANT_F4_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.449247699162001246087072559021, 0.390605475879083674506375700730)
#define LIBCOLOUR_ILLUMINANT_F5_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.319739939104951298443069163113, 0.342367055369128092667807550242)
#define LIBCOLOUR_ILLUMINANT_F6_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.386626908526034762658696308790, 0.378372201588893453116924092683)
#define LIBCOLOUR_ILLUMINANT_F7_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.315645637312390425766039925293, 0.329508145132134222521358424274)
#define LIBCOLOUR_ILLUMINANT_F8_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.348965563721531868424108324689, 0.359317299140994528272585739614)
#define LIBCOLOUR_ILLUMINANT_F9_10DEG_OBS   LIBCOLOUR_ILLUMINANT__(0.378258900384649654480284652891, 0.370371375730762564248976786985)
#define LIBCOLOUR_ILLUMINANT_F10_10DEG_OBS  LIBCOLOUR_ILLUMINANT__(0.350893389986753234666139178444, 0.354302210111646531665030579461)
#define LIBCOLOUR_ILLUMINANT_F11_10DEG_OBS  LIBCOLOUR_ILLUMINANT__(0.385435391037903751776383387551, 0.371094786781121399599214782938)
#define LIBCOLOUR_ILLUMINANT_F12_10DEG_OBS  LIBCOLOUR_ILLUMINANT__(0.442654456042513022584472537346, 0.397060737666593277506166259627)


#define LIBCOLOUR_LIST_MODELS\
  X(LIBCOLOUR_RGB, libcolour_rgb_t)\
  X(LIBCOLOUR_SRGB, libcolour_srgb_t)\
  X(LIBCOLOUR_CIEXYY, libcolour_ciexyy_t)\
  X(LIBCOLOUR_CIEXYZ, libcolour_ciexyz_t)\
  X(LIBCOLOUR_CIELAB, libcolour_cielab_t)\
  X(LIBCOLOUR_CIELUV, libcolour_cieluv_t)\
  X(LIBCOLOUR_CIELCHUV, libcolour_cielchuv_t)\
  X(LIBCOLOUR_YIQ, libcolour_yiq_t)\
  X(LIBCOLOUR_YDBDR, libcolour_ydbdr_t)\
  X(LIBCOLOUR_YUV, libcolour_yuv_t)\
  X(LIBCOLOUR_YPBPR, libcolour_ypbpr_t)\
  X(LIBCOLOUR_YCGCO, libcolour_ycgco_t)\
  X(LIBCOLOUR_CIE1960UCS, libcolour_cie1960ucs_t)\
  X(LIBCOLOUR_CIEUVW, libcolour_cieuvw_t)


typedef enum libcolour_model {
#define X(C, T) C,
  LIBCOLOUR_LIST_MODELS
#undef X
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
  LIBCOLOUR_RGB_COLOUR_SPACE_ECI_RGB,
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

typedef struct libcolour_cielchuv {
  enum libcolour_model model;
  double L;
  double C;
  double h;
  struct libcolour_ciexyz white;
  double one_revolution;
} libcolour_cielchuv_t;

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
  double (*to_encoded_red)(double);
  double (*to_decoded_red)(double);
  double (*to_encoded_green)(double);
  double (*to_decoded_green)(double);
  double (*to_encoded_blue)(double);
  double (*to_decoded_blue)(double);
  struct libcolour_ciexyy red;
  struct libcolour_ciexyy green;
  struct libcolour_ciexyy blue;
  struct libcolour_ciexyy white;
  double white_r;
  double white_g;
  double white_b;
  double M[3][3];
  double Minv[3][3];
  enum libcolour_rgb_colour_space colour_space;
} libcolour_rgb_t;


typedef union libcolour_colour {
  enum libcolour_model model;
  struct libcolour_rgb rgb;
  struct libcolour_srgb srgb;
  struct libcolour_ciexyy ciexyy;
  struct libcolour_ciexyz ciexyz;
  struct libcolour_cielab cielab;
  struct libcolour_cieluv cieluv;
  struct libcolour_cielchuv cielchuv;
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
int libcolour_get_rgb_colour_space(libcolour_rgb_t*, libcolour_rgb_colour_space_t);
size_t libcolour_marshal(const libcolour_colour_t*, void*);
size_t libcolour_unmarshal(libcolour_colour_t*, const void*);



#endif

