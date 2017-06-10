/* See LICENSE file for copyright and license details. */
#include "libcolour.h"

#include <alloca.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define libcolour_colour_t     libcolour_colour_lf_t
#define libcolour_rgb_t        libcolour_rgb_lf_t
#define libcolour_srgb_t       libcolour_srgb_lf_t
#define libcolour_ciexyy_t     libcolour_ciexyy_lf_t
#define libcolour_ciexyz_t     libcolour_ciexyz_lf_t
#define libcolour_cielab_t     libcolour_cielab_lf_t
#define libcolour_cieluv_t     libcolour_cieluv_lf_t
#define libcolour_cielchuv_t   libcolour_cielchuv_lf_t
#define libcolour_yiq_t        libcolour_yiq_lf_t
#define libcolour_ydbdr_t      libcolour_ydbdr_lf_t
#define libcolour_yuv_t        libcolour_yuv_lf_t
#define libcolour_ypbpr_t      libcolour_ypbpr_lf_t
#define libcolour_ycgco_t      libcolour_ycgco_lf_t
#define libcolour_cie1960ucs_t libcolour_cie1960ucs_lf_t
#define libcolour_cieuvw_t     libcolour_cieuvw_lf_t

#define TYPE double
#define LIST_MODELS(X) LIBCOLOUR_LIST_MODELS(X, lf)
#define D(X) ((TYPE)(X))

#define LIBCOLOUR_ILLUMINANT_A   LIBCOLOUR_ILLUMINANT_A_LF
#define LIBCOLOUR_ILLUMINANT_B   LIBCOLOUR_ILLUMINANT_B_LF
#define LIBCOLOUR_ILLUMINANT_C   LIBCOLOUR_ILLUMINANT_C_LF
#define LIBCOLOUR_ILLUMINANT_D50 LIBCOLOUR_ILLUMINANT_D50_LF
#define LIBCOLOUR_ILLUMINANT_D55 LIBCOLOUR_ILLUMINANT_D55_LF
#define LIBCOLOUR_ILLUMINANT_D65 LIBCOLOUR_ILLUMINANT_D65_LF
#define LIBCOLOUR_ILLUMINANT_D75 LIBCOLOUR_ILLUMINANT_D75_LF
#define LIBCOLOUR_ILLUMINANT_E   LIBCOLOUR_ILLUMINANT_E_LF
#define LIBCOLOUR_ILLUMINANT_F1  LIBCOLOUR_ILLUMINANT_F1_LF
#define LIBCOLOUR_ILLUMINANT_F2  LIBCOLOUR_ILLUMINANT_F2_LF
#define LIBCOLOUR_ILLUMINANT_F3  LIBCOLOUR_ILLUMINANT_F3_LF
#define LIBCOLOUR_ILLUMINANT_F4  LIBCOLOUR_ILLUMINANT_F4_LF
#define LIBCOLOUR_ILLUMINANT_F5  LIBCOLOUR_ILLUMINANT_F5_LF
#define LIBCOLOUR_ILLUMINANT_F6  LIBCOLOUR_ILLUMINANT_F6_LF
#define LIBCOLOUR_ILLUMINANT_F7  LIBCOLOUR_ILLUMINANT_F7_LF
#define LIBCOLOUR_ILLUMINANT_F8  LIBCOLOUR_ILLUMINANT_F8_LF
#define LIBCOLOUR_ILLUMINANT_F9  LIBCOLOUR_ILLUMINANT_F9_LF
#define LIBCOLOUR_ILLUMINANT_F10 LIBCOLOUR_ILLUMINANT_F10_LF
#define LIBCOLOUR_ILLUMINANT_F11 LIBCOLOUR_ILLUMINANT_F11_LF
#define LIBCOLOUR_ILLUMINANT_F12 LIBCOLOUR_ILLUMINANT_F12_LF

#define xisinf(X)    (isinf(X))
#define xisnan(X)    (isnan(X))
#define xpow(X, Y)   (pow((X), (Y)))
#define xcbrt(X)     (cbrt(X))
#define xsqrt(X)     (sqrt(X))
#define xlog(X)      (log(X))
#define xexp(X)      (exp(X))
#define xsin(X)      (sin(X))
#define xcos(X)      (cos(X))
#define xatan2(X, Y) (atan2((X), (Y)))

#undef libcolour_convert
#undef libcolour_srgb_encode
#undef libcolour_srgb_decode
#undef libcolour_delta_e
#undef libcolour_proper
#undef libcolour_get_rgb_colour_space
#undef libcolour_marshal
#undef libcolour_unmarshal
#undef libcolour_convert_en_masse

#define libcolour_convert              libcolour_convert_lf
#define libcolour_srgb_encode          libcolour_srgb_encode_lf
#define libcolour_srgb_decode          libcolour_srgb_decode_lf
#define libcolour_delta_e              libcolour_delta_e_lf
#define libcolour_proper               libcolour_proper_lf
#define libcolour_get_rgb_colour_space libcolour_get_rgb_colour_space_lf
#define libcolour_marshal              libcolour_marshal_lf
#define libcolour_unmarshal            libcolour_unmarshal_lf
#define libcolour_convert_en_masse     libcolour_convert_en_masse_lf

#include "common.h"
#include "convert-template.c"
#include "libcolour-template.c"
#include "en_masse-template.c"
