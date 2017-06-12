/* See LICENSE file for copyright and license details. */
#include "libcolour.h"

#include <alloca.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define libcolour_colour_t     libcolour_colour_f_t
#define libcolour_rgb_t        libcolour_rgb_f_t
#define libcolour_srgb_t       libcolour_srgb_f_t
#define libcolour_ciexyy_t     libcolour_ciexyy_f_t
#define libcolour_ciexyz_t     libcolour_ciexyz_f_t
#define libcolour_cielab_t     libcolour_cielab_f_t
#define libcolour_cieluv_t     libcolour_cieluv_f_t
#define libcolour_cielchuv_t   libcolour_cielchuv_f_t
#define libcolour_yiq_t        libcolour_yiq_f_t
#define libcolour_ydbdr_t      libcolour_ydbdr_f_t
#define libcolour_yuv_t        libcolour_yuv_f_t
#define libcolour_ypbpr_t      libcolour_ypbpr_f_t
#define libcolour_ycgco_t      libcolour_ycgco_f_t
#define libcolour_cie1960ucs_t libcolour_cie1960ucs_f_t
#define libcolour_cieuvw_t     libcolour_cieuvw_f_t

#define TYPE float
#define LIST_MODELS(X) LIBCOLOUR_LIST_MODELS(X, f)
#define D(X) ((TYPE)(X))

#define LIBCOLOUR_ILLUMINANT_A   LIBCOLOUR_ILLUMINANT_A_F
#define LIBCOLOUR_ILLUMINANT_B   LIBCOLOUR_ILLUMINANT_B_F
#define LIBCOLOUR_ILLUMINANT_C   LIBCOLOUR_ILLUMINANT_C_F
#define LIBCOLOUR_ILLUMINANT_D50 LIBCOLOUR_ILLUMINANT_D50_F
#define LIBCOLOUR_ILLUMINANT_D55 LIBCOLOUR_ILLUMINANT_D55_F
#define LIBCOLOUR_ILLUMINANT_D65 LIBCOLOUR_ILLUMINANT_D65_F
#define LIBCOLOUR_ILLUMINANT_D75 LIBCOLOUR_ILLUMINANT_D75_F
#define LIBCOLOUR_ILLUMINANT_E   LIBCOLOUR_ILLUMINANT_E_F
#define LIBCOLOUR_ILLUMINANT_F1  LIBCOLOUR_ILLUMINANT_F1_F
#define LIBCOLOUR_ILLUMINANT_F2  LIBCOLOUR_ILLUMINANT_F2_F
#define LIBCOLOUR_ILLUMINANT_F3  LIBCOLOUR_ILLUMINANT_F3_F
#define LIBCOLOUR_ILLUMINANT_F4  LIBCOLOUR_ILLUMINANT_F4_F
#define LIBCOLOUR_ILLUMINANT_F5  LIBCOLOUR_ILLUMINANT_F5_F
#define LIBCOLOUR_ILLUMINANT_F6  LIBCOLOUR_ILLUMINANT_F6_F
#define LIBCOLOUR_ILLUMINANT_F7  LIBCOLOUR_ILLUMINANT_F7_F
#define LIBCOLOUR_ILLUMINANT_F8  LIBCOLOUR_ILLUMINANT_F8_F
#define LIBCOLOUR_ILLUMINANT_F9  LIBCOLOUR_ILLUMINANT_F9_F
#define LIBCOLOUR_ILLUMINANT_F10 LIBCOLOUR_ILLUMINANT_F10_F
#define LIBCOLOUR_ILLUMINANT_F11 LIBCOLOUR_ILLUMINANT_F11_F
#define LIBCOLOUR_ILLUMINANT_F12 LIBCOLOUR_ILLUMINANT_F12_F

#define xisinf(X)    (isinff(X))
#define xisnan(X)    (isnanf(X))
#define xpow(X, Y)   (powf((X), (Y)))
#define xcbrt(X)     (cbrtf(X))
#define xsqrt(X)     (sqrtf(X))
#define xlog(X)      (logf(X))
#define xexp(X)      (expf(X))
#define xsin(X)      (sinf(X))
#define xcos(X)      (cosf(X))
#define xatan2(X, Y) (atan2f((X), (Y)))

#undef libcolour_convert
#undef libcolour_srgb_encode
#undef libcolour_srgb_decode
#undef libcolour_delta_e
#undef libcolour_proper
#undef libcolour_get_rgb_colour_space
#undef libcolour_marshal
#undef libcolour_unmarshal
#undef libcolour_convert_en_masse

#define libcolour_convert              libcolour_convert_f
#define libcolour_srgb_encode          libcolour_srgb_encode_f
#define libcolour_srgb_decode          libcolour_srgb_decode_f
#define libcolour_delta_e              libcolour_delta_e_f
#define libcolour_proper               libcolour_proper_f
#define libcolour_get_rgb_colour_space libcolour_get_rgb_colour_space_f
#define libcolour_marshal              libcolour_marshal_f
#define libcolour_unmarshal            libcolour_unmarshal_f
#define libcolour_convert_en_masse     libcolour_convert_en_masse_f

#include "common.h"
#include "conversions.h"
#include "libcolour-template.c"
#include "en_masse-template.c"
